#include "ui/InputManager.h"
#include <gtest/gtest.h>
#include <GLFW/glfw3.h>
#include <memory>

using namespace RebelCAD::UI;

class InputManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize GLFW
        if (!glfwInit()) {
            FAIL() << "Failed to initialize GLFW";
        }

        // Create test window
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window for testing
        window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            FAIL() << "Failed to create GLFW window";
        }

        // Initialize input manager
        manager = &InputManager::getInstance();
        manager->initialize(window);
    }

    void TearDown() override {
        manager->shutdown();
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    // Helper to simulate key events
    void simulateKeyEvent(int key, int action) {
        glfwSetWindowUserPointer(window, manager);
        InputManager::keyCallback(window, key, 0, action, 0);
        manager->update();
    }

    // Helper to simulate mouse button events
    void simulateMouseButtonEvent(int button, int action) {
        glfwSetWindowUserPointer(window, manager);
        InputManager::mouseButtonCallback(window, button, action, 0);
        manager->update();
    }

    // Helper to simulate mouse movement
    void simulateMouseMove(double x, double y) {
        glfwSetWindowUserPointer(window, manager);
        InputManager::cursorPosCallback(window, x, y);
        manager->update();
    }

    // Helper to simulate scroll
    void simulateScroll(double xoffset, double yoffset) {
        glfwSetWindowUserPointer(window, manager);
        InputManager::scrollCallback(window, xoffset, yoffset);
        manager->update();
    }

    GLFWwindow* window;
    InputManager* manager;
};

TEST_F(InputManagerTest, KeyPressAndRelease) {
    // Simulate key press
    simulateKeyEvent(GLFW_KEY_A, GLFW_PRESS);
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_A));
    EXPECT_TRUE(manager->isKeyHeld(GLFW_KEY_A));
    EXPECT_FALSE(manager->isKeyReleased(GLFW_KEY_A));

    // Update should clear the pressed state but maintain held
    manager->update();
    EXPECT_FALSE(manager->isKeyPressed(GLFW_KEY_A));
    EXPECT_TRUE(manager->isKeyHeld(GLFW_KEY_A));
    EXPECT_FALSE(manager->isKeyReleased(GLFW_KEY_A));

    // Simulate key release
    simulateKeyEvent(GLFW_KEY_A, GLFW_RELEASE);
    EXPECT_FALSE(manager->isKeyPressed(GLFW_KEY_A));
    EXPECT_FALSE(manager->isKeyHeld(GLFW_KEY_A));
    EXPECT_TRUE(manager->isKeyReleased(GLFW_KEY_A));
}

TEST_F(InputManagerTest, MouseButtonStates) {
    // Simulate mouse button press
    simulateMouseButtonEvent(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
    EXPECT_TRUE(manager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT));
    EXPECT_TRUE(manager->isMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT));
    EXPECT_FALSE(manager->isMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT));

    // Update should clear the pressed state but maintain held
    manager->update();
    EXPECT_FALSE(manager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT));
    EXPECT_TRUE(manager->isMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT));
    EXPECT_FALSE(manager->isMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT));

    // Simulate mouse button release
    simulateMouseButtonEvent(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
    EXPECT_FALSE(manager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT));
    EXPECT_FALSE(manager->isMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT));
    EXPECT_TRUE(manager->isMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT));
}

TEST_F(InputManagerTest, MouseMovement) {
    // Simulate mouse movement
    simulateMouseMove(100.0, 200.0);
    
    double x, y;
    manager->getMousePosition(x, y);
    EXPECT_DOUBLE_EQ(x, 100.0);
    EXPECT_DOUBLE_EQ(y, 200.0);

    // Simulate another movement to test delta
    simulateMouseMove(150.0, 250.0);
    
    double dx, dy;
    manager->getMouseDelta(dx, dy);
    EXPECT_DOUBLE_EQ(dx, 50.0);
    EXPECT_DOUBLE_EQ(dy, 50.0);
}

TEST_F(InputManagerTest, ScrollInput) {
    // Simulate scroll
    simulateScroll(0.0, 1.0);
    EXPECT_DOUBLE_EQ(manager->getMouseScrollDelta(), 1.0);

    // Update should reset scroll delta
    manager->update();
    EXPECT_DOUBLE_EQ(manager->getMouseScrollDelta(), 0.0);
}

TEST_F(InputManagerTest, FocusManagement) {
    // Test focus setting and querying
    manager->setFocusedWidget("widget1");
    EXPECT_EQ(manager->getFocusedWidget(), "widget1");
    EXPECT_TRUE(manager->hasFocus("widget1"));
    EXPECT_FALSE(manager->hasFocus("widget2"));

    // Test focus change
    manager->setFocusedWidget("widget2");
    EXPECT_EQ(manager->getFocusedWidget(), "widget2");
    EXPECT_FALSE(manager->hasFocus("widget1"));
    EXPECT_TRUE(manager->hasFocus("widget2"));
}

TEST_F(InputManagerTest, InputMapping) {
    bool actionTriggered = false;
    manager->mapKeyToAction(GLFW_KEY_SPACE, "jump", [&]() {
        actionTriggered = true;
    });

    // Simulate mapped key press
    simulateKeyEvent(GLFW_KEY_SPACE, GLFW_PRESS);
    EXPECT_TRUE(actionTriggered);

    // Test unmapping
    actionTriggered = false;
    manager->unmapKey(GLFW_KEY_SPACE);
    simulateKeyEvent(GLFW_KEY_SPACE, GLFW_PRESS);
    EXPECT_FALSE(actionTriggered);
}

TEST_F(InputManagerTest, MultipleKeyMappings) {
    int actionCount = 0;
    manager->mapKeyToAction(GLFW_KEY_A, "action1", [&]() { actionCount++; });
    manager->mapKeyToAction(GLFW_KEY_B, "action1", [&]() { actionCount++; });

    // Trigger action by name should trigger both callbacks
    manager->triggerAction("action1");
    EXPECT_EQ(actionCount, 2);
}

TEST_F(InputManagerTest, InputStateClearing) {
    // Simulate key press
    simulateKeyEvent(GLFW_KEY_A, GLFW_PRESS);
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_A));

    // Update should clear pressed state
    manager->update();
    EXPECT_FALSE(manager->isKeyPressed(GLFW_KEY_A));
    EXPECT_TRUE(manager->isKeyHeld(GLFW_KEY_A));

    // Release should clear held state
    simulateKeyEvent(GLFW_KEY_A, GLFW_RELEASE);
    EXPECT_FALSE(manager->isKeyHeld(GLFW_KEY_A));
    EXPECT_TRUE(manager->isKeyReleased(GLFW_KEY_A));

    // Update should clear released state
    manager->update();
    EXPECT_FALSE(manager->isKeyReleased(GLFW_KEY_A));
}

TEST_F(InputManagerTest, NonexistentStates) {
    // Test querying states that haven't been set
    EXPECT_FALSE(manager->isKeyPressed(GLFW_KEY_X));
    EXPECT_FALSE(manager->isKeyHeld(GLFW_KEY_X));
    EXPECT_FALSE(manager->isKeyReleased(GLFW_KEY_X));
    EXPECT_FALSE(manager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE));
    EXPECT_FALSE(manager->isMouseButtonHeld(GLFW_MOUSE_BUTTON_MIDDLE));
    EXPECT_FALSE(manager->isMouseButtonReleased(GLFW_MOUSE_BUTTON_MIDDLE));
}
