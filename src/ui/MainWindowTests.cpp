#include "ui/MainWindow.h"
#include <gtest/gtest.h>

namespace RebelCAD {
namespace UI {
namespace Tests {

class MainWindowTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // Cleanup code
    }
};

TEST_F(MainWindowTests, Constructor) {
    MainWindow window("Test Window", 800, 600);
    EXPECT_EQ(window.getTitle(), "Test Window");
    EXPECT_EQ(window.getWidth(), 800);
    EXPECT_EQ(window.getHeight(), 600);
}

TEST_F(MainWindowTests, SetSize) {
    MainWindow window;
    window.setSize(1024, 768);
    EXPECT_EQ(window.getWidth(), 1024);
    EXPECT_EQ(window.getHeight(), 768);
}

TEST_F(MainWindowTests, SetTitle) {
    MainWindow window;
    window.setTitle("New Title");
    EXPECT_EQ(window.getTitle(), "New Title");
}

TEST_F(MainWindowTests, InitializeShutdown) {
    MainWindow window;
    EXPECT_NO_THROW(window.initialize());
    EXPECT_NO_THROW(window.shutdown());
}

TEST_F(MainWindowTests, ShouldCloseInitiallyFalse) {
    MainWindow window;
    EXPECT_FALSE(window.shouldClose());
}

TEST_F(MainWindowTests, MultipleInitializeCalls) {
    MainWindow window;
    EXPECT_NO_THROW(window.initialize());
    EXPECT_NO_THROW(window.initialize()); // Should handle gracefully
    EXPECT_NO_THROW(window.shutdown());
}

TEST_F(MainWindowTests, MultipleShutdownCalls) {
    MainWindow window;
    EXPECT_NO_THROW(window.initialize());
    EXPECT_NO_THROW(window.shutdown());
    EXPECT_NO_THROW(window.shutdown()); // Should handle gracefully
}

} // namespace Tests
} // namespace UI
} // namespace RebelCAD
