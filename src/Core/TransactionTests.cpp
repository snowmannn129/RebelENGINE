#include <gtest/gtest.h>
#include "core/Transaction.h"
#include "core/TransactionManager.h"
#include "core/OperationQueue.h"
#include <memory>

namespace RebelCAD {
namespace Core {
namespace Tests {

// Mock operation for testing
class MockOperation : public Operation {
public:
    explicit MockOperation(bool rollbackSupported = true)
        : m_executed(false)
        , m_rolledBack(false)
        , m_rollbackSupported(rollbackSupported) {}

    bool execute() override {
        m_executed = true;
        return true;
    }

    bool rollback() override {
        if (!m_rollbackSupported) return false;
        m_rolledBack = true;
        return true;
    }

    bool isRollbackSupported() const override {
        return m_rollbackSupported;
    }

    bool wasExecuted() const { return m_executed; }
    bool wasRolledBack() const { return m_rolledBack; }

private:
    bool m_executed;
    bool m_rolledBack;
    bool m_rollbackSupported;
};

class TransactionTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_opQueue = std::make_unique<OperationQueue>(1); // Single worker
        m_transactionManager = std::make_unique<TransactionManager>(*m_opQueue);
    }

    std::unique_ptr<OperationQueue> m_opQueue;
    std::unique_ptr<TransactionManager> m_transactionManager;
};

TEST_F(TransactionTest, CreateTransaction) {
    auto transaction = m_transactionManager->beginTransaction("Test Transaction");
    ASSERT_NE(transaction, nullptr);
    EXPECT_EQ(transaction->getName(), "Test Transaction");
    EXPECT_EQ(transaction->getState(), Transaction::State::Pending);
    EXPECT_TRUE(transaction->getOperations().empty());
}

TEST_F(TransactionTest, AddOperationToTransaction) {
    auto transaction = m_transactionManager->beginTransaction("Test Transaction");
    auto op = std::make_shared<MockOperation>();
    
    EXPECT_TRUE(transaction->addOperation(op));
    EXPECT_EQ(transaction->getOperations().size(), 1);
    EXPECT_EQ(transaction->getOperations()[0], op);
}

TEST_F(TransactionTest, CommitTransaction) {
    auto transaction = m_transactionManager->beginTransaction("Test Transaction");
    auto op1 = std::make_shared<MockOperation>();
    auto op2 = std::make_shared<MockOperation>();
    
    transaction->addOperation(op1);
    transaction->addOperation(op2);
    
    EXPECT_TRUE(m_transactionManager->commitTransaction(transaction));
    
    EXPECT_EQ(transaction->getState(), Transaction::State::Committed);
    EXPECT_TRUE(op1->wasExecuted());
    EXPECT_TRUE(op2->wasExecuted());
    EXPECT_FALSE(op1->wasRolledBack());
    EXPECT_FALSE(op2->wasRolledBack());
}

TEST_F(TransactionTest, RollbackTransaction) {
    auto transaction = m_transactionManager->beginTransaction("Test Transaction");
    auto op1 = std::make_shared<MockOperation>();
    auto op2 = std::make_shared<MockOperation>();
    
    transaction->addOperation(op1);
    transaction->addOperation(op2);
    
    EXPECT_TRUE(m_transactionManager->rollbackTransaction(transaction));
    
    EXPECT_EQ(transaction->getState(), Transaction::State::RolledBack);
    EXPECT_TRUE(op1->wasRolledBack());
    EXPECT_TRUE(op2->wasRolledBack());
}

TEST_F(TransactionTest, TransactionHistory) {
    auto transaction1 = m_transactionManager->beginTransaction("Transaction 1");
    auto transaction2 = m_transactionManager->beginTransaction("Transaction 2");
    
    transaction1->addOperation(std::make_shared<MockOperation>());
    transaction2->addOperation(std::make_shared<MockOperation>());
    
    m_transactionManager->commitTransaction(transaction1);
    m_transactionManager->commitTransaction(transaction2);
    
    auto history = m_transactionManager->getTransactionHistory();
    EXPECT_EQ(history.size(), 2);
    EXPECT_EQ(history[0]->getName(), "Transaction 1");
    EXPECT_EQ(history[1]->getName(), "Transaction 2");
}

TEST_F(TransactionTest, NonRollbackableOperation) {
    auto transaction = m_transactionManager->beginTransaction("Test Transaction");
    auto op = std::make_shared<MockOperation>(false); // Non-rollbackable
    
    transaction->addOperation(op);
    
    // Should fail since operation doesn't support rollback
    EXPECT_FALSE(transaction->isRollbackable());
    EXPECT_FALSE(m_transactionManager->rollbackTransaction(transaction));
}

TEST_F(TransactionTest, TransactionDependencies) {
    auto transaction = m_transactionManager->beginTransaction("Test Transaction");
    auto op1 = std::make_shared<MockOperation>();
    auto op2 = std::make_shared<MockOperation>();
    auto op3 = std::make_shared<MockOperation>();
    
    transaction->addOperation(op1);
    transaction->addOperation(op2);
    transaction->addOperation(op3);
    
    EXPECT_TRUE(m_transactionManager->commitTransaction(transaction));
    
    // Operations should have executed in order
    EXPECT_TRUE(op1->wasExecuted());
    EXPECT_TRUE(op2->wasExecuted());
    EXPECT_TRUE(op3->wasExecuted());
}

} // namespace Tests
} // namespace Core
} // namespace RebelCAD
