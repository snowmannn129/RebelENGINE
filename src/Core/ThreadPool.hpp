#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "MemoryPool.hpp"
#include "EventSystem.hpp"

namespace rebel_cad::core {

/**
 * @brief Thread-safe task queue with priority support
 * @tparam T Task type that will be stored in the queue
 */
template<typename T>
class ThreadSafeQueue {
public:
    /**
     * @brief Push a task into the queue
     * @param task Task to be pushed
     * @param priority Priority level (higher number = higher priority)
     */
    void push(T&& task, int priority = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.emplace(std::make_pair(priority, std::move(task)));
        cv_.notify_one();
    }

    /**
     * @brief Try to pop a task from the queue
     * @param task Reference to store the popped task
     * @return true if task was popped, false if queue is empty
     */
    bool tryPop(T& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tasks_.empty()) {
            return false;
        }
        task = std::move(tasks_.top().second);
        tasks_.pop();
        return true;
    }

    /**
     * @brief Wait and pop a task from the queue
     * @param task Reference to store the popped task
     */
    void waitAndPop(T& task) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !tasks_.empty(); });
        task = std::move(tasks_.top().second);
        tasks_.pop();
    }

    /**
     * @brief Check if queue is empty
     * @return true if queue is empty
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return tasks_.empty();
    }

private:
    using PriorityTask = std::pair<int, T>;
    struct ComparePriority {
        bool operator()(const PriorityTask& lhs, const PriorityTask& rhs) {
            return lhs.first < rhs.first;
        }
    };

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::priority_queue<PriorityTask, std::vector<PriorityTask>, ComparePriority> tasks_;
};

/**
 * @brief Thread pool for managing and executing tasks asynchronously
 * Integrates with RebelCAD's memory management and event systems
 */
class ThreadPool {
public:
    /**
     * @brief Construct a thread pool
     * @param numThreads Number of worker threads (defaults to hardware concurrency)
     */
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
        : stop_(false) {
        try {
            for (size_t i = 0; i < numThreads; ++i) {
                workers_.emplace_back(&ThreadPool::workerFunction, this);
            }
        } catch (...) {
            stop_ = true;
            throw;
        }
    }

    /**
     * @brief Destructor ensures all threads are properly joined
     */
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    /**
     * @brief Submit a task to be executed by the thread pool
     * @tparam F Function type
     * @tparam Args Argument types
     * @param priority Task priority (higher number = higher priority)
     * @param f Function to execute
     * @param args Arguments for the function
     * @return std::future containing the result of the task
     */
    template<typename F, typename... Args>
    auto submit(int priority, F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using ReturnType = typename std::invoke_result<F, Args...>::type;
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("Cannot submit task to stopped ThreadPool");
            }
            tasks_.push([task]() { (*task)(); }, priority);
        }
        cv_.notify_one();
        return result;
    }

    /**
     * @brief Get the number of worker threads
     * @return Number of threads in the pool
     */
    size_t threadCount() const {
        return workers_.size();
    }

    /**
     * @brief Check if thread pool has pending tasks
     * @return true if there are pending tasks
     */
    bool hasPendingTasks() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return !tasks_.empty();
    }

    /**
     * @brief Wait for all current tasks to complete
     */
    void waitForCompletion() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        completion_cv_.wait(lock, [this] { 
            return tasks_.empty() && (active_tasks_ == 0);
        });
    }

private:
    /**
     * @brief Worker thread function that processes tasks from the queue
     */
    void workerFunction() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this] { 
                    return stop_ || !tasks_.empty(); 
                });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                if (!tasks_.tryPop(task)) {
                    continue;
                }
                ++active_tasks_;
            }

            try {
                task();
            } catch (const std::exception& e) {
                // Log error through event system
                auto event = std::make_shared<SystemEvent>();
                event->type = SystemEvent::Type::Error;
                event->message = std::string("ThreadPool error: ") + e.what();
                event->category = EventSystem::Category::System;
                event->priority = EventSystem::Priority::High;
                EventSystem().publish(event);
            }

            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                --active_tasks_;
                if (tasks_.empty() && active_tasks_ == 0) {
                    completion_cv_.notify_all();
                }
            }
        }
    }

    std::vector<std::thread> workers_;
    ThreadSafeQueue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::condition_variable completion_cv_;
    std::atomic<bool> stop_;
    std::atomic<size_t> active_tasks_{0};
};

} // namespace rebel_cad::core
