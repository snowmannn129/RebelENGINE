#pragma once

#include "Operation.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace RebelCAD {
namespace Core {

/**
 * @brief Manages the execution of operations in RebelCAD
 * 
 * The OperationQueue provides:
 * - Priority-based operation scheduling
 * - Async operation support with worker threads
 * - Operation dependency management
 * - Progress tracking and status updates
 * - Cancellation support
 */
class OperationQueue {
public:
    /**
     * @brief Constructor
     * @param numWorkers Number of worker threads for async operations
     */
    explicit OperationQueue(size_t numWorkers = 2);
    
    /**
     * @brief Destructor - ensures clean shutdown of worker threads
     */
    ~OperationQueue();

    // Prevent copying
    OperationQueue(const OperationQueue&) = delete;
    OperationQueue& operator=(const OperationQueue&) = delete;

    /**
     * @brief Enqueue an operation for execution
     * @param operation The operation to execute
     * @param dependencies List of operations that must complete before this one
     * @return true if operation was successfully queued
     */
    bool enqueue(OperationPtr operation, 
                const std::vector<OperationPtr>& dependencies = {});

    /**
     * @brief Cancel a pending or running operation
     * @param operation The operation to cancel
     * @return true if cancellation was initiated
     */
    bool cancel(OperationPtr operation);

    /**
     * @brief Cancel all pending and running operations
     */
    void cancelAll();

    /**
     * @brief Get the number of pending operations
     */
    size_t getPendingCount() const;

    /**
     * @brief Get the number of running operations
     */
    size_t getRunningCount() const;

    /**
     * @brief Check if the queue is currently processing operations
     */
    bool isProcessing() const;

    /**
     * @brief Wait for all current operations to complete
     * @param timeout_ms Maximum time to wait in milliseconds (0 = wait forever)
     * @return true if all operations completed, false if timeout occurred
     */
    bool waitForCompletion(uint32_t timeout_ms = 0);

    /**
     * @brief Set callback for when operations are added/removed/updated
     */
    using QueueChangeCallback = std::function<void()>;
    void setQueueChangeCallback(QueueChangeCallback callback);

private:
    // Internal operation queue entry
    struct QueueEntry {
        OperationPtr operation;
        std::unordered_set<OperationPtr> dependencies;
        std::unordered_set<OperationPtr> dependents;
    };

    // Custom comparator for priority queue
    struct QueueEntryCompare {
        bool operator()(const QueueEntry& a, const QueueEntry& b) const {
            return static_cast<int>(a.operation->getPriority()) < 
                   static_cast<int>(b.operation->getPriority());
        }
    };

    void workerThread();
    bool canExecute(const QueueEntry& entry) const;
    void notifyQueueChanged();
    void cleanup();

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryCompare> m_queue;
    std::unordered_map<OperationPtr, QueueEntry> m_runningOps;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_running;
    
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    QueueChangeCallback m_queueChangeCallback;
};

} // namespace Core
} // namespace RebelCAD
