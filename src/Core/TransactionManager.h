#pragma once

#include "Transaction.h"
#include "OperationQueue.h"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <deque>

namespace RebelCAD {
namespace Core {

/**
 * @brief Manages transactions and their lifecycle in RebelCAD
 * 
 * The TransactionManager coordinates with the OperationQueue to ensure
 * atomic execution of operations within transactions. It handles:
 * - Transaction creation and tracking
 * - Commit/rollback coordination
 * - Transaction history management
 * - Recovery from failed transactions
 */
class TransactionManager {
public:
    /**
     * @brief Constructor
     * @param opQueue Reference to the operation queue to use
     */
    explicit TransactionManager(OperationQueue& opQueue);

    /**
     * @brief Begin a new transaction
     * @param name Descriptive name for the transaction
     * @return Shared pointer to the new transaction
     */
    TransactionPtr beginTransaction(const std::string& name);

    /**
     * @brief Commit a transaction
     * @param transaction Transaction to commit
     * @return true if commit was successful
     */
    bool commitTransaction(TransactionPtr transaction);

    /**
     * @brief Roll back a transaction
     * @param transaction Transaction to roll back
     * @return true if rollback was successful
     */
    bool rollbackTransaction(TransactionPtr transaction);

    /**
     * @brief Get a transaction by ID
     * @param id Transaction ID to look up
     * @return Shared pointer to transaction or nullptr if not found
     */
    TransactionPtr getTransaction(const std::string& id) const;

    /**
     * @brief Get all active transactions
     * @return Vector of active transaction pointers
     */
    std::vector<TransactionPtr> getActiveTransactions() const;

    /**
     * @brief Get transaction history
     * @param maxEntries Maximum number of history entries to return (0 = all)
     * @return Vector of completed transaction pointers
     */
    std::vector<TransactionPtr> getTransactionHistory(size_t maxEntries = 0) const;

private:
    // Maximum number of completed transactions to keep in history
    static constexpr size_t MAX_HISTORY_SIZE = 1000;

    // Internal helper methods
    void addToHistory(TransactionPtr transaction);
    void cleanupHistory();
    bool validateTransaction(TransactionPtr transaction) const;
    
    OperationQueue& m_operationQueue;
    std::unordered_map<std::string, TransactionPtr> m_activeTransactions;
    std::deque<TransactionPtr> m_transactionHistory;
    
    mutable std::mutex m_mutex;
};

} // namespace Core
} // namespace RebelCAD
