#include "core/TransactionManager.h"
#include <sstream>
#include <algorithm>

namespace RebelCAD {
namespace Core {

TransactionManager::TransactionManager(OperationQueue& opQueue)
    : m_operationQueue(opQueue) {
}

TransactionPtr TransactionManager::beginTransaction(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Create unique ID based on name and timestamp
    std::stringstream ss;
    ss << name << "_" << std::chrono::system_clock::now().time_since_epoch().count();
    std::string id = ss.str();
    
    // Create and store transaction
    auto transaction = std::make_shared<Transaction>(name);
    m_activeTransactions[id] = transaction;
    
    return transaction;
}

bool TransactionManager::commitTransaction(TransactionPtr transaction) {
    if (!validateTransaction(transaction)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Set transaction to running state
    transaction->setState(Transaction::State::Running);
    
    // Queue all operations with dependencies
    for (size_t i = 0; i < transaction->getOperations().size(); ++i) {
        auto op = transaction->getOperations()[i];
        
        // Each operation depends on previous operations in the transaction
        std::vector<OperationPtr> dependencies;
        if (i > 0) {
            dependencies.insert(
                dependencies.end(),
                transaction->getOperations().begin(),
                transaction->getOperations().begin() + i
            );
        }
        
        if (!m_operationQueue.enqueue(op, dependencies)) {
            // If enqueueing fails, rollback and fail transaction
            rollbackTransaction(transaction);
            return false;
        }
    }
    
    // Wait for all operations to complete
    if (!m_operationQueue.waitForCompletion()) {
        rollbackTransaction(transaction);
        return false;
    }
    
    // Check if all operations completed successfully
    bool success = std::all_of(
        transaction->getOperations().begin(),
        transaction->getOperations().end(),
        [](const OperationPtr& op) {
            return op->getState() == OperationState::Completed;
        });
    
    if (!success) {
        rollbackTransaction(transaction);
        return false;
    }
    
    // Commit transaction
    if (!transaction->commit()) {
        rollbackTransaction(transaction);
        return false;
    }
    
    // Move to history
    addToHistory(transaction);
    return true;
}

bool TransactionManager::rollbackTransaction(TransactionPtr transaction) {
    if (!validateTransaction(transaction)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Cancel any pending operations
    for (const auto& op : transaction->getOperations()) {
        if (op->getState() == OperationState::Pending ||
            op->getState() == OperationState::Running) {
            m_operationQueue.cancel(op);
        }
    }
    
    // Wait for cancellations to complete
    m_operationQueue.waitForCompletion();
    
    // Rollback transaction
    bool success = transaction->rollback();
    
    // Move to history
    addToHistory(transaction);
    return success;
}

TransactionPtr TransactionManager::getTransaction(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_activeTransactions.find(id);
    if (it != m_activeTransactions.end()) {
        return it->second;
    }
    
    // Search history if not found in active transactions
    auto histIt = std::find_if(
        m_transactionHistory.begin(),
        m_transactionHistory.end(),
        [&id](const TransactionPtr& t) {
            return t->getName() == id;
        });
    
    return (histIt != m_transactionHistory.end()) ? *histIt : nullptr;
}

std::vector<TransactionPtr> TransactionManager::getActiveTransactions() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<TransactionPtr> transactions;
    transactions.reserve(m_activeTransactions.size());
    
    for (const auto& pair : m_activeTransactions) {
        transactions.push_back(pair.second);
    }
    
    return transactions;
}

std::vector<TransactionPtr> TransactionManager::getTransactionHistory(
    size_t maxEntries) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (maxEntries == 0 || maxEntries > m_transactionHistory.size()) {
        maxEntries = m_transactionHistory.size();
    }
    
    std::vector<TransactionPtr> history;
    history.reserve(maxEntries);
    
    auto start = m_transactionHistory.end() - maxEntries;
    history.insert(history.end(), start, m_transactionHistory.end());
    
    return history;
}

void TransactionManager::addToHistory(TransactionPtr transaction) {
    // Remove from active transactions
    for (auto it = m_activeTransactions.begin(); it != m_activeTransactions.end(); ++it) {
        if (it->second == transaction) {
            m_activeTransactions.erase(it);
            break;
        }
    }
    
    // Add to history
    m_transactionHistory.push_back(transaction);
    
    // Cleanup if needed
    cleanupHistory();
}

void TransactionManager::cleanupHistory() {
    while (m_transactionHistory.size() > MAX_HISTORY_SIZE) {
        m_transactionHistory.pop_front();
    }
}

bool TransactionManager::validateTransaction(TransactionPtr transaction) const {
    if (!transaction) {
        return false;
    }
    
    // Check if transaction is in our active set
    auto it = std::find_if(
        m_activeTransactions.begin(),
        m_activeTransactions.end(),
        [&transaction](const auto& pair) {
            return pair.second == transaction;
        });
    
    return it != m_activeTransactions.end();
}

} // namespace Core
} // namespace RebelCAD
