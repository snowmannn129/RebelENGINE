#pragma once

#include "Command.h"
#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace RebelCAD {
namespace Core {

/**
 * @brief Represents the current state of an operation
 */
enum class OperationState {
    Pending,    ///< Operation is queued but not started
    Running,    ///< Operation is currently executing
    Completed,  ///< Operation completed successfully
    Failed,     ///< Operation failed to complete
    Cancelled   ///< Operation was cancelled
};

/**
 * @brief Priority levels for operations
 */
enum class OperationPriority {
    Low,        ///< Background operations (e.g., auto-save)
    Normal,     ///< Standard user operations
    High,       ///< Time-sensitive operations
    Critical    ///< Must execute immediately
};

/**
 * @brief Base class for all operations in RebelCAD
 * 
 * Operations extend Commands by adding:
 * - Priority levels for execution ordering
 * - Progress tracking
 * - Cancellation support
 * - Asynchronous execution capability
 * - Dependency management
 */
class Operation : public Command {
    friend class OperationQueue;
public:
    using ProgressCallback = std::function<void(float)>;
    using CompletionCallback = std::function<void(bool)>;

    Operation(OperationPriority priority = OperationPriority::Normal)
        : m_priority(priority)
        , m_state(OperationState::Pending)
        , m_progress(0.0f)
        , m_startTime()
        , m_endTime() {}

    virtual ~Operation() = default;

    /**
     * @brief Check if the operation can be executed asynchronously
     * @return true if the operation supports async execution
     */
    virtual bool isAsync() const { return false; }

    /**
     * @brief Check if the operation can be cancelled
     * @return true if the operation supports cancellation
     */
    virtual bool isCancellable() const { return false; }

    /**
     * @brief Check if the operation supports rollback
     * @return true if the operation can be rolled back
     */
    virtual bool isRollbackSupported() const { return false; }

    /**
     * @brief Roll back the operation's changes
     * @return true if rollback was successful
     */
    virtual bool rollback() { return false; }

    /**
     * @brief Request operation cancellation
     * @return true if cancellation was initiated
     */
    virtual bool cancel() { return false; }

    /**
     * @brief Get the operation's current progress (0.0 to 1.0)
     */
    float getProgress() const { return m_progress; }

    /**
     * @brief Get the operation's current state
     */
    OperationState getState() const { return m_state; }

    /**
     * @brief Get the operation's priority level
     */
    OperationPriority getPriority() const { return m_priority; }

    /**
     * @brief Get the operation's execution duration
     * @return Duration in milliseconds, or 0 if not started/completed
     */
    int64_t getDurationMs() const {
        if (m_startTime.time_since_epoch().count() == 0) return 0;
        auto endTime = (m_endTime.time_since_epoch().count() == 0) ? 
            std::chrono::steady_clock::now() : m_endTime;
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - m_startTime).count();
    }

    /**
     * @brief Set a callback for progress updates
     */
    void setProgressCallback(ProgressCallback callback) {
        m_progressCallback = callback;
    }

    /**
     * @brief Set a callback for operation completion
     */
    void setCompletionCallback(CompletionCallback callback) {
        m_completionCallback = callback;
    }

protected:
    /**
     * @brief Update the operation's progress
     * @param progress Progress value between 0.0 and 1.0
     */
    void updateProgress(float progress) {
        m_progress = std::max(0.0f, std::min(1.0f, progress));
        if (m_progressCallback) {
            m_progressCallback(m_progress);
        }
    }

    /**
     * @brief Update the operation's state
     */
    void setState(OperationState state) {
        m_state = state;
        if (state == OperationState::Running && 
            m_startTime.time_since_epoch().count() == 0) {
            m_startTime = std::chrono::steady_clock::now();
        }
        else if (state == OperationState::Completed || 
                 state == OperationState::Failed ||
                 state == OperationState::Cancelled) {
            m_endTime = std::chrono::steady_clock::now();
            if (m_completionCallback) {
                m_completionCallback(state == OperationState::Completed);
            }
        }
    }

private:
    OperationPriority m_priority;
    OperationState m_state;
    float m_progress;
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_endTime;
    ProgressCallback m_progressCallback;
    CompletionCallback m_completionCallback;
};

// Smart pointer type for Operations
using OperationPtr = std::shared_ptr<Operation>;

} // namespace Core
} // namespace RebelCAD
