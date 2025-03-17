#include <gtest/gtest.h>
#include <chrono>
#include <atomic>
#include <thread>

#include "core/ThreadPool.hpp"
#include "core/TaskScheduler.hpp"
#include "core/EventSystem.hpp"

using namespace rebel_cad::core;
using namespace std::chrono_literals;

class ThreadingTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Subscribe to system events for testing
        eventSystem.subscribe(EventSystem::Category::System, 
            [this](const std::shared_ptr<EventSystem::Event>& event) {
                if (auto taskEvent = std::dynamic_pointer_cast<TaskEvent>(event)) {
                    taskEvents.push_back(taskEvent);
                }
            });
    }

    void TearDown() override {
        taskEvents.clear();
    }

    EventSystem eventSystem;
    std::vector<std::shared_ptr<TaskEvent>> taskEvents;
};

TEST_F(ThreadingTests, ThreadPoolBasicExecution) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    
    // Submit multiple tasks
    auto future1 = pool.submit(1, [&counter]() {
        std::this_thread::sleep_for(100ms);
        counter++;
    });
    
    auto future2 = pool.submit(1, [&counter]() {
        std::this_thread::sleep_for(50ms);
        counter++;
    });

    // Wait for completion
    future1.wait();
    future2.wait();
    
    EXPECT_EQ(counter, 2);
}

TEST_F(ThreadingTests, ThreadPoolPriorityHandling) {
    ThreadPool pool(1); // Single thread to ensure priority ordering
    std::vector<int> execution;
    std::mutex mutex;

    // Submit tasks with different priorities
    pool.submit(1, [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        execution.push_back(1);
    });

    pool.submit(3, [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        execution.push_back(3);
    });

    pool.submit(2, [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        execution.push_back(2);
    });

    // Wait for all tasks to complete
    pool.waitForCompletion();

    // Higher priority tasks should execute first
    EXPECT_EQ(execution[0], 3);
    EXPECT_EQ(execution[1], 2);
    EXPECT_EQ(execution[2], 1);
}

TEST_F(ThreadingTests, TaskSchedulerDependencyHandling) {
    TaskScheduler scheduler(2);
    std::vector<std::string> executionOrder;
    std::mutex mutex;

    // Create tasks with dependencies
    auto task1 = scheduler.scheduleTask("task1", [&]() {
        std::this_thread::sleep_for(50ms);
        std::lock_guard<std::mutex> lock(mutex);
        executionOrder.push_back("task1");
    });

    auto task2 = scheduler.scheduleTask("task2", [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        executionOrder.push_back("task2");
    });

    auto task3 = scheduler.scheduleTask("task3", [&]() {
        std::lock_guard<std::mutex> lock(mutex);
        executionOrder.push_back("task3");
    });

    // Set up dependencies: task2 depends on task1, task3 depends on task2
    scheduler.addDependency(task2, task1);
    scheduler.addDependency(task3, task2);

    // Start execution
    scheduler.start();
    scheduler.waitForCompletion();

    // Verify execution order
    ASSERT_EQ(executionOrder.size(), 3);
    EXPECT_EQ(executionOrder[0], "task1");
    EXPECT_EQ(executionOrder[1], "task2");
    EXPECT_EQ(executionOrder[2], "task3");
}

TEST_F(ThreadingTests, TaskSchedulerEventEmission) {
    TaskScheduler scheduler(1);

    // Schedule a task that will succeed
    auto successTask = scheduler.scheduleTask("success", []() {
        std::this_thread::sleep_for(50ms);
    });

    // Schedule a task that will fail
    auto failTask = scheduler.scheduleTask("fail", []() {
        throw std::runtime_error("Task failed");
    });

    // Start execution and wait
    scheduler.start();
    
    // Wait a bit for events to be processed
    std::this_thread::sleep_for(200ms);

    // Verify events
    bool foundCreated = false;
    bool foundStarted = false;
    bool foundCompleted = false;
    bool foundFailed = false;

    for (const auto& event : taskEvents) {
        if (event->taskId == "success") {
            switch (event->type) {
                case TaskEvent::Type::Created:
                    foundCreated = true;
                    break;
                case TaskEvent::Type::Started:
                    foundStarted = true;
                    break;
                case TaskEvent::Type::Completed:
                    foundCompleted = true;
                    break;
                default:
                    break;
            }
        } else if (event->taskId == "fail") {
            if (event->type == TaskEvent::Type::Failed) {
                foundFailed = true;
                EXPECT_EQ(event->details, "Task failed");
            }
        }
    }

    EXPECT_TRUE(foundCreated);
    EXPECT_TRUE(foundStarted);
    EXPECT_TRUE(foundCompleted);
    EXPECT_TRUE(foundFailed);
}

TEST_F(ThreadingTests, TaskSchedulerConcurrentExecution) {
    TaskScheduler scheduler(4);
    std::atomic<int> counter{0};
    constexpr int NUM_TASKS = 100;

    // Schedule many independent tasks
    std::vector<std::shared_ptr<Task>> tasks;
    for (int i = 0; i < NUM_TASKS; ++i) {
        tasks.push_back(scheduler.scheduleTask(
            "task" + std::to_string(i),
            [&counter]() {
                std::this_thread::sleep_for(1ms);
                counter++;
            }
        ));
    }

    // Start execution
    scheduler.start();
    scheduler.waitForCompletion();

    // Verify all tasks completed
    EXPECT_EQ(counter, NUM_TASKS);
}

TEST_F(ThreadingTests, ThreadPoolErrorHandling) {
    ThreadPool pool(1);
    bool exceptionCaught = false;

    // Submit a task that throws
    auto future = pool.submit(1, []() {
        throw std::runtime_error("Test error");
    });

    // Exception should be propagated through the future
    try {
        future.get();
    } catch (const std::runtime_error& e) {
        exceptionCaught = true;
        EXPECT_STREQ(e.what(), "Test error");
    }

    EXPECT_TRUE(exceptionCaught);
}

TEST_F(ThreadingTests, TaskSchedulerCancellation) {
    TaskScheduler scheduler(1);
    std::atomic<bool> taskRan{false};

    // Create a chain of dependent tasks
    auto task1 = scheduler.scheduleTask("task1", [&]() {
        std::this_thread::sleep_for(100ms);
        taskRan = true;
    });

    auto task2 = scheduler.scheduleTask("task2", [&]() {
        taskRan = true;
    });

    scheduler.addDependency(task2, task1);

    // Start execution
    scheduler.start();

    // Verify task state
    auto task = scheduler.getTask("task2");
    ASSERT_NE(task, nullptr);
    EXPECT_EQ(task->getState(), TaskState::Pending);

    scheduler.waitForCompletion();
    EXPECT_TRUE(taskRan);
}
