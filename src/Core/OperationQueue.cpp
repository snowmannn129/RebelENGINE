#include "core/OperationQueue.h"
#include <algorithm>
#include <chrono>

namespace RebelCAD {
namespace Core {

OperationQueue::OperationQueue(size_t numWorkers)
    : m_running(true) {
    // Start worker threads
    for (size_t i = 0; i < numWorkers; ++i) {
        m_workers.emplace_back(&OperationQueue::workerThread, this);
    }
}

OperationQueue::~OperationQueue() {
    // Signal shutdown and wake all workers
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
    }
    m_condition.notify_all();

    // Wait for all workers to finish
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

bool OperationQueue::enqueue(OperationPtr operation,
                           const std::vector<OperationPtr>& dependencies) {
    if (!operation) return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Create queue entry
    QueueEntry entry;
    entry.operation = operation;
    entry.dependencies.insert(dependencies.begin(), dependencies.end());

    // Update dependent relationships
    for (const auto& dep : dependencies) {
        auto it = m_runningOps.find(dep);
        if (it != m_runningOps.end()) {
            it->second.dependents.insert(operation);
        }
    }

    // Add to queue
    m_queue.push(entry);
    notifyQueueChanged();
    m_condition.notify_one();
    
    return true;
}

bool OperationQueue::cancel(OperationPtr operation) {
    if (!operation) return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if operation is running
    auto runIt = m_runningOps.find(operation);
    if (runIt != m_runningOps.end()) {
        if (operation->isCancellable()) {
            operation->cancel();
            return true;
        }
        return false;
    }

    // Remove from queue if pending
    std::vector<QueueEntry> temp;
    while (!m_queue.empty()) {
        auto entry = m_queue.top();
        m_queue.pop();
        
        if (entry.operation != operation) {
            temp.push_back(entry);
        }
    }

    // Rebuild queue without cancelled operation
    for (const auto& entry : temp) {
        m_queue.push(entry);
    }

    notifyQueueChanged();
    return true;
}

void OperationQueue::cancelAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Cancel running operations
    for (const auto& pair : m_runningOps) {
        if (pair.first->isCancellable()) {
            pair.first->cancel();
        }
    }

    // Clear pending queue
    while (!m_queue.empty()) {
        m_queue.pop();
    }

    notifyQueueChanged();
}

size_t OperationQueue::getPendingCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

size_t OperationQueue::getRunningCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_runningOps.size();
}

bool OperationQueue::isProcessing() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_queue.empty() || !m_runningOps.empty();
}

bool OperationQueue::waitForCompletion(uint32_t timeout_ms) {
    auto start = std::chrono::steady_clock::now();
    
    while (isProcessing()) {
        if (timeout_ms > 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - start).count();
            if (elapsed >= timeout_ms) {
                return false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return true;
}

void OperationQueue::setQueueChangeCallback(QueueChangeCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queueChangeCallback = callback;
}

void OperationQueue::workerThread() {
    while (true) {
        QueueEntry entry;
        
        // Wait for work
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] {
                return !m_running || 
                       (!m_queue.empty() && canExecute(m_queue.top()));
            });

            if (!m_running) break;

            // Get next operation
            entry = m_queue.top();
            m_queue.pop();
            
            // Move to running set
            m_runningOps[entry.operation] = entry;
            notifyQueueChanged();
        }

        // Execute operation
        entry.operation->setState(OperationState::Running);
        bool success = entry.operation->execute();
        entry.operation->setState(
            success ? OperationState::Completed : OperationState::Failed);

        // Cleanup after completion
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            cleanup();
            notifyQueueChanged();
        }
    }
}

bool OperationQueue::canExecute(const QueueEntry& entry) const {
    // Check all dependencies are complete
    return std::all_of(
        entry.dependencies.begin(),
        entry.dependencies.end(),
        [](const OperationPtr& dep) {
            return dep->getState() == OperationState::Completed;
        });
}

void OperationQueue::notifyQueueChanged() {
    if (m_queueChangeCallback) {
        m_queueChangeCallback();
    }
}

void OperationQueue::cleanup() {
    // Remove completed operations and update dependencies
    auto it = m_runningOps.begin();
    while (it != m_runningOps.end()) {
        const auto& entry = it->second;
        if (entry.operation->getState() == OperationState::Completed ||
            entry.operation->getState() == OperationState::Failed ||
            entry.operation->getState() == OperationState::Cancelled) {
            
            // Remove this operation from others' dependencies
            for (const auto& dependent : entry.dependents) {
                auto queueIt = m_runningOps.find(dependent);
                if (queueIt != m_runningOps.end()) {
                    queueIt->second.dependencies.erase(entry.operation);
                }
            }
            
            it = m_runningOps.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace Core
} // namespace RebelCAD
