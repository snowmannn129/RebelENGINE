#include "core/OperationQueue.h"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

namespace RebelCAD {
namespace Core {
namespace Tests {

// Mock operation for testing
class MockOperation : public Operation {
public:
    MockOperation(bool async = false, bool cancellable = false,
                 OperationPriority priority = OperationPriority::Normal,
                 std::chrono::milliseconds duration = std::chrono::milliseconds(0))
        : Operation(priority)
        , m_async(async)
        , m_cancellable(cancellable)
        , m_duration(duration)
        , m_cancelled(false) {}

    bool execute() override {
        if (m_duration.count() > 0) {
            std::this_thread::sleep_for(m_duration);
        }
        return !m_cancelled;
    }

    bool undo() override { return true; }
    bool redo() override { return true; }
    std::string getDescription() const override { return "Mock Operation"; }

    bool isAsync() const override { return m_async; }
    bool isCancellable() const override { return m_cancellable; }
    bool cancel() override {
        if (m_cancellable) {
            m_cancelled = true;
            return true;
        }
        return false;
    }

private:
    bool m_async;
    bool m_cancellable;
    std::chrono::milliseconds m_duration;
    bool m_cancelled;
};

TEST(OperationQueueTests, EnqueueAndExecute) {
    OperationQueue queue(1);
    auto op = std::make_shared<MockOperation>();
    
    EXPECT_TRUE(queue.enqueue(op));
    EXPECT_EQ(1, queue.getPendingCount());
    
    // Wait for completion
    EXPECT_TRUE(queue.waitForCompletion(1000));
    EXPECT_EQ(OperationState::Completed, op->getState());
    EXPECT_EQ(0, queue.getPendingCount());
}

TEST(OperationQueueTests, PriorityOrdering) {
    OperationQueue queue(1);
    std::vector<OperationState> executionOrder;
    
    auto lowOp = std::make_shared<MockOperation>(
        false, false, OperationPriority::Low);
    auto highOp = std::make_shared<MockOperation>(
        false, false, OperationPriority::High);
    auto normalOp = std::make_shared<MockOperation>(
        false, false, OperationPriority::Normal);
    
    // Set completion callbacks to track execution order
    auto trackExecution = [&executionOrder](OperationPtr op) {
        op->setCompletionCallback([&executionOrder, op](bool) {
            executionOrder.push_back(op->getState());
        });
    };
    
    trackExecution(lowOp);
    trackExecution(highOp);
    trackExecution(normalOp);
    
    // Enqueue in reverse priority order
    queue.enqueue(lowOp);
    queue.enqueue(highOp);
    queue.enqueue(normalOp);
    
    EXPECT_TRUE(queue.waitForCompletion(1000));
    
    // Verify high priority executed first
    EXPECT_EQ(OperationState::Completed, highOp->getState());
    EXPECT_EQ(OperationState::Completed, normalOp->getState());
    EXPECT_EQ(OperationState::Completed, lowOp->getState());
}

TEST(OperationQueueTests, DependencyHandling) {
    OperationQueue queue(1);
    
    auto op1 = std::make_shared<MockOperation>();
    auto op2 = std::make_shared<MockOperation>();
    auto op3 = std::make_shared<MockOperation>();
    
    // op3 depends on op2, which depends on op1
    queue.enqueue(op3, {op2});
    queue.enqueue(op2, {op1});
    queue.enqueue(op1);
    
    EXPECT_TRUE(queue.waitForCompletion(1000));
    
    // Verify all completed in correct order
    EXPECT_EQ(OperationState::Completed, op1->getState());
    EXPECT_EQ(OperationState::Completed, op2->getState());
    EXPECT_EQ(OperationState::Completed, op3->getState());
}

TEST(OperationQueueTests, CancellationHandling) {
    OperationQueue queue(1);
    
    auto op = std::make_shared<MockOperation>(
        false, true, OperationPriority::Normal,
        std::chrono::milliseconds(100));
    
    queue.enqueue(op);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_TRUE(queue.cancel(op));
    EXPECT_TRUE(queue.waitForCompletion(1000));
    EXPECT_EQ(OperationState::Cancelled, op->getState());
}

TEST(OperationQueueTests, QueueChangeCallback) {
    OperationQueue queue(1);
    int callbackCount = 0;
    
    queue.setQueueChangeCallback([&callbackCount]() {
        callbackCount++;
    });
    
    auto op = std::make_shared<MockOperation>();
    queue.enqueue(op);
    
    EXPECT_TRUE(queue.waitForCompletion(1000));
    EXPECT_GT(callbackCount, 0);
}

TEST(OperationQueueTests, AsyncOperations) {
    OperationQueue queue(2);
    
    auto asyncOp1 = std::make_shared<MockOperation>(
        true, false, OperationPriority::Normal,
        std::chrono::milliseconds(100));
    auto asyncOp2 = std::make_shared<MockOperation>(
        true, false, OperationPriority::Normal,
        std::chrono::milliseconds(100));
    
    queue.enqueue(asyncOp1);
    queue.enqueue(asyncOp2);
    
    // Both should run concurrently
    EXPECT_TRUE(queue.waitForCompletion(150));
    EXPECT_EQ(OperationState::Completed, asyncOp1->getState());
    EXPECT_EQ(OperationState::Completed, asyncOp2->getState());
}

} // namespace Tests
} // namespace Core
} // namespace RebelCAD
