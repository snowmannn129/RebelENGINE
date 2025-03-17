#pragma once

#include "Operation.h"
#include <vector>
#include <memory>
#include <string>
#include <chrono>

namespace RebelCAD {
namespace Core {

/**
 * @brief Represents a group of operations that should be executed atomically
 * 
 * A Transaction ensures that either all operations succeed or none of them do.
 * If any operation in the transaction fails, all previously completed operations
 * will be rolled back to maintain data consistency.
 */
class Transaction {
public:
    enum class State {
        Pending,    // Transaction created but not started
        Running,    // Transaction is currently executing
        Committed,  // All operations completed successfully
        RolledBack, // Transaction was rolled back due to failure
        Failed     // Transaction failed and couldn't be rolled back
    };

    /**
     * @brief Create a new transaction
     * @param name Descriptive name for the transaction
     */
    explicit Transaction(const std::string& name);

    /**
     * @brief Add an operation to this transaction
     * @param operation Operation to add
     * @return true if operation was added successfully
     */
    bool addOperation(OperationPtr operation);

    /**
     * @brief Get the current state of the transaction
     */
    State getState() const { return m_state; }

    /**
     * @brief Get the transaction name
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Get the time this transaction was created
     */
    std::chrono::system_clock::time_point getCreationTime() const { return m_creationTime; }

    /**
     * @brief Get all operations in this transaction
     */
    const std::vector<OperationPtr>& getOperations() const { return m_operations; }

    /**
     * @brief Check if this transaction can be rolled back
     */
    bool isRollbackable() const;

private:
    friend class TransactionManager;

    // Called by TransactionManager
    bool commit();
    bool rollback();
    void setState(State state) { m_state = state; }

    std::string m_name;
    std::vector<OperationPtr> m_operations;
    State m_state;
    std::chrono::system_clock::time_point m_creationTime;
};

using TransactionPtr = std::shared_ptr<Transaction>;

} // namespace Core
} // namespace RebelCAD
