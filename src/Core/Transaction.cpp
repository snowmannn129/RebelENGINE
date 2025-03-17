#include "core/Transaction.h"
#include <algorithm>

namespace RebelCAD {
namespace Core {

Transaction::Transaction(const std::string& name)
    : m_name(name)
    , m_state(State::Pending)
    , m_creationTime(std::chrono::system_clock::now()) {
}

bool Transaction::addOperation(OperationPtr operation) {
    if (!operation || m_state != State::Pending) {
        return false;
    }

    // Check if operation already exists in transaction
    auto it = std::find_if(m_operations.begin(), m_operations.end(),
        [&operation](const OperationPtr& op) {
            return op == operation;
        });

    if (it != m_operations.end()) {
        return false;
    }

    m_operations.push_back(operation);
    return true;
}

bool Transaction::isRollbackable() const {
    // Check if all operations support rollback
    return std::all_of(m_operations.begin(), m_operations.end(),
        [](const OperationPtr& op) {
            return op->isRollbackSupported();
        });
}

bool Transaction::commit() {
    if (m_state != State::Running) {
        return false;
    }

    // All operations should have completed successfully at this point
    // Just update state
    m_state = State::Committed;
    return true;
}

bool Transaction::rollback() {
    if (m_state != State::Running || !isRollbackable()) {
        return false;
    }

    // Rollback operations in reverse order
    bool success = true;
    for (auto it = m_operations.rbegin(); it != m_operations.rend(); ++it) {
        if (!(*it)->rollback()) {
            success = false;
            break;
        }
    }

    m_state = success ? State::RolledBack : State::Failed;
    return success;
}

} // namespace Core
} // namespace RebelCAD
