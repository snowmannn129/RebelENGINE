#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <future>
#include <shared_mutex>

#include "ThreadPool.hpp"
#include "EventSystem.hpp"

namespace rebel_cad::core {

/**
 * @brief Represents a task's current execution state
 */
enum class TaskState {
    Pending,    // Task is waiting for execution
    Ready,      // Task is ready to execute (all dependencies met)
    Running,    // Task is currently executing
    Completed,  // Task has completed successfully
    Failed,     // Task failed during execution
    Cancelled   // Task was cancelled before completion
};

/**
 * @brief Task event for tracking task lifecycle
 */
struct TaskEvent : public EventSystem::Event {
    enum class Type {
        Created,
        DependencyResolved,
        Started,
        Completed,
        Failed,
        Cancelled
    };
    Type type;
    std::string taskId;
    std::string details;
};

/**
 * @brief Represents a scheduled task with dependencies and priority
 */
class Task {
public:
    using TaskPtr = std::shared_ptr<Task>;
    using TaskResult = std::future<void>;
    
    /**
     * @brief Construct a new Task
     * @param id Unique task identifier
     * @param func Function to execute
     * @param priority Task priority (higher number = higher priority)
     */
    template<typename F>
    Task(std::string id, F&& func, int priority = 0)
        : id_(std::move(id))
        , func_(std::forward<F>(func))
        , priority_(priority)
        , state_(TaskState::Pending) {}

    /**
     * @brief Add a dependency that must complete before this task
     * @param dependency Task that must complete first
     */
    void addDependency(TaskPtr dependency) {
        std::lock_guard<std::mutex> lock(mutex_);
        dependencies_.push_back(dependency);
    }

    /**
     * @brief Get the task's unique identifier
     */
    const std::string& getId() const { return id_; }

    /**
     * @brief Get the task's current state
     */
    TaskState getState() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }

    /**
     * @brief Get the task's priority
     */
    int getPriority() const { return priority_; }

    /**
     * @brief Check if all dependencies are completed
     */
    bool areDependenciesMet() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::all_of(dependencies_.begin(), dependencies_.end(),
            [](const TaskPtr& dep) {
                return dep->getState() == TaskState::Completed;
            });
    }

    /**
     * @brief Execute the task
     * @param eventSystem Reference to event system for status updates
     */
    void execute(EventSystem& eventSystem) {
        try {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                state_ = TaskState::Running;
            }
            
            // Emit started event
            auto startEvent = std::make_shared<TaskEvent>();
            startEvent->type = TaskEvent::Type::Started;
            startEvent->taskId = id_;
            startEvent->category = EventSystem::Category::System;
            startEvent->priority = EventSystem::Priority::Normal;
            eventSystem.publish(startEvent);

            // Execute task
            func_();

            {
                std::lock_guard<std::mutex> lock(mutex_);
                state_ = TaskState::Completed;
            }

            // Emit completed event
            auto completeEvent = std::make_shared<TaskEvent>();
            completeEvent->type = TaskEvent::Type::Completed;
            completeEvent->taskId = id_;
            completeEvent->category = EventSystem::Category::System;
            completeEvent->priority = EventSystem::Priority::Normal;
            eventSystem.publish(completeEvent);

        } catch (const std::exception& e) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                state_ = TaskState::Failed;
            }

            // Emit failed event
            auto failEvent = std::make_shared<TaskEvent>();
            failEvent->type = TaskEvent::Type::Failed;
            failEvent->taskId = id_;
            failEvent->details = e.what();
            failEvent->category = EventSystem::Category::System;
            failEvent->priority = EventSystem::Priority::High;
            eventSystem.publish(failEvent);

            throw; // Rethrow to notify scheduler
        }
    }

private:
    std::string id_;
    std::function<void()> func_;
    int priority_;
    TaskState state_;
    std::vector<TaskPtr> dependencies_;
    mutable std::mutex mutex_;
};

/**
 * @brief Manages task scheduling and execution with dependency handling
 */
class TaskScheduler {
public:
    /**
     * @brief Construct a new Task Scheduler
     * @param threadCount Number of worker threads (defaults to hardware concurrency)
     */
    explicit TaskScheduler(size_t threadCount = std::thread::hardware_concurrency())
        : threadPool_(threadCount)
        , eventSystem_() {}

    /**
     * @brief Schedule a new task
     * @param id Unique task identifier
     * @param func Function to execute
     * @param priority Task priority
     * @return Shared pointer to the created task
     */
    template<typename F>
    std::shared_ptr<Task> scheduleTask(std::string id, F&& func, int priority = 0) {
        std::lock_guard<std::shared_mutex> lock(mutex_);
        
        auto task = std::make_shared<Task>(std::move(id), std::forward<F>(func), priority);
        tasks_[task->getId()] = task;

        // Emit task created event
        auto event = std::make_shared<TaskEvent>();
        event->type = TaskEvent::Type::Created;
        event->taskId = task->getId();
        event->category = EventSystem::Category::System;
        event->priority = EventSystem::Priority::Normal;
        eventSystem_.publish(event);

        return task;
    }

    /**
     * @brief Add a dependency between tasks
     * @param task Task that depends on another
     * @param dependency Task that must complete first
     */
    void addDependency(std::shared_ptr<Task> task, std::shared_ptr<Task> dependency) {
        task->addDependency(dependency);
    }

    /**
     * @brief Start executing all scheduled tasks
     */
    void start() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        // Submit all ready tasks to thread pool
        for (const auto& [id, task] : tasks_) {
            if (task->getState() == TaskState::Pending && task->areDependenciesMet()) {
                submitTask(task);
            }
        }
    }

    /**
     * @brief Wait for all tasks to complete
     */
    void waitForCompletion() {
        threadPool_.waitForCompletion();
    }

    /**
     * @brief Get a task by its ID
     * @param id Task identifier
     * @return Shared pointer to task if found, nullptr otherwise
     */
    std::shared_ptr<Task> getTask(const std::string& id) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = tasks_.find(id);
        return (it != tasks_.end()) ? it->second : nullptr;
    }

private:
    /**
     * @brief Submit a task to the thread pool
     * @param task Task to submit
     */
    void submitTask(std::shared_ptr<Task> task) {
        threadPool_.submit(task->getPriority(), [this, task]() {
            task->execute(eventSystem_);
            
            // Check for dependent tasks that are now ready
            std::shared_lock<std::shared_mutex> lock(mutex_);
            for (const auto& [id, dependent] : tasks_) {
                if (dependent->getState() == TaskState::Pending && 
                    dependent->areDependenciesMet()) {
                    submitTask(dependent);
                }
            }
        });
    }

    ThreadPool threadPool_;
    EventSystem eventSystem_;
    std::unordered_map<std::string, std::shared_ptr<Task>> tasks_;
    mutable std::shared_mutex mutex_;
};

} // namespace rebel_cad::core
