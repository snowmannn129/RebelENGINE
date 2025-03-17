#include <gtest/gtest.h>
#include "core/UndoRedoManager.h"
#include "core/UndoRedoEvent.h"
#include <memory>

namespace RebelCAD {
namespace Core {
namespace Tests {

// Simple test command that modifies a value
class TestCommand : public Command {
public:
    TestCommand(int& value, int newValue, const std::string& description)
        : value_(value)
        , oldValue_(value)
        , newValue_(newValue)
        , description_(description) {}

    bool execute() override {
        value_ = newValue_;
        return true;
    }

    bool undo() override {
        value_ = oldValue_;
        return true;
    }

    bool redo() override {
        return execute();
    }

    std::string getDescription() const override {
        return description_;
    }

private:
    int& value_;
    int oldValue_;
    int newValue_;
    std::string description_;
};

// Test fixture
class UndoRedoManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<UndoRedoManager>();
        testValue = 0;
    }

    std::unique_ptr<UndoRedoManager> manager;
    int testValue;
};

TEST_F(UndoRedoManagerTest, ExecuteCommand) {
    auto cmd = std::make_shared<TestCommand>(testValue, 5, "Set value to 5");
    EXPECT_TRUE(manager->executeCommand(cmd));
    EXPECT_EQ(testValue, 5);
    EXPECT_TRUE(manager->canUndo());
    EXPECT_FALSE(manager->canRedo());
}

TEST_F(UndoRedoManagerTest, UndoCommand) {
    auto cmd = std::make_shared<TestCommand>(testValue, 5, "Set value to 5");
    manager->executeCommand(cmd);
    EXPECT_TRUE(manager->undo());
    EXPECT_EQ(testValue, 0);
    EXPECT_FALSE(manager->canUndo());
    EXPECT_TRUE(manager->canRedo());
}

TEST_F(UndoRedoManagerTest, RedoCommand) {
    auto cmd = std::make_shared<TestCommand>(testValue, 5, "Set value to 5");
    manager->executeCommand(cmd);
    manager->undo();
    EXPECT_TRUE(manager->redo());
    EXPECT_EQ(testValue, 5);
    EXPECT_TRUE(manager->canUndo());
    EXPECT_FALSE(manager->canRedo());
}

TEST_F(UndoRedoManagerTest, CommandGroup) {
    manager->beginCommandGroup("Multiple changes");
    
    auto cmd1 = std::make_shared<TestCommand>(testValue, 5, "Set value to 5");
    auto cmd2 = std::make_shared<TestCommand>(testValue, 10, "Set value to 10");
    
    EXPECT_TRUE(manager->executeCommand(cmd1));
    EXPECT_TRUE(manager->executeCommand(cmd2));
    
    manager->endCommandGroup();
    
    EXPECT_EQ(testValue, 10);
    EXPECT_TRUE(manager->canUndo());
    
    // Undo should revert both commands
    manager->undo();
    EXPECT_EQ(testValue, 0);
}

TEST_F(UndoRedoManagerTest, ClearHistory) {
    auto cmd = std::make_shared<TestCommand>(testValue, 5, "Set value to 5");
    manager->executeCommand(cmd);
    manager->clear();
    
    EXPECT_FALSE(manager->canUndo());
    EXPECT_FALSE(manager->canRedo());
}

TEST_F(UndoRedoManagerTest, GetDescriptions) {
    auto cmd = std::make_shared<TestCommand>(testValue, 5, "Set value to 5");
    manager->executeCommand(cmd);
    
    EXPECT_EQ(manager->getUndoDescription(), "Set value to 5");
    EXPECT_EQ(manager->getRedoDescription(), "");
    
    manager->undo();
    EXPECT_EQ(manager->getUndoDescription(), "");
    EXPECT_EQ(manager->getRedoDescription(), "Set value to 5");
}

// Test event handling
class EventListener {
public:
    void onUndoRedoStateChanged(Event& e) {
        auto& event = static_cast<UndoRedoStateChangedEvent&>(e);
        lastCanUndo = event.canUndo();
        lastCanRedo = event.canRedo();
        lastUndoDesc = event.getUndoDescription();
        lastRedoDesc = event.getRedoDescription();
        eventCount++;
    }

    bool lastCanUndo = false;
    bool lastCanRedo = false;
    std::string lastUndoDesc;
    std::string lastRedoDesc;
    int eventCount = 0;
};

TEST_F(UndoRedoManagerTest, EventNotification) {
    EventListener listener;
    auto subscription = EventBus::getInstance().subscribe<UndoRedoStateChangedEvent>(
        [&](Event& e) { listener.onUndoRedoStateChanged(e); }
    );

    auto cmd = std::make_shared<TestCommand>(testValue, 5, "Set value to 5");
    manager->executeCommand(cmd);

    EXPECT_TRUE(listener.lastCanUndo);
    EXPECT_FALSE(listener.lastCanRedo);
    EXPECT_EQ(listener.lastUndoDesc, "Set value to 5");
    EXPECT_EQ(listener.lastRedoDesc, "");
    EXPECT_GT(listener.eventCount, 0);

    EventBus::getInstance().unsubscribe(subscription);
}

} // namespace Tests
} // namespace Core
} // namespace RebelCAD
