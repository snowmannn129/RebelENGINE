#include <gtest/gtest.h>
#include <GLFW/glfw3.h>
#include "graphics/CameraController.h"
#include "graphics/Viewport.h"

using namespace RebelCAD::Graphics;

class CameraControllerTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Create window for testing
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Hidden window
        window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        viewport = std::make_unique<Viewport>(800, 600);
        controller = std::make_unique<CameraController>(viewport.get(), window);
    }

    void TearDown() override {
        controller.reset();
        viewport.reset();
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    GLFWwindow* window;
    std::unique_ptr<Viewport> viewport;
    std::unique_ptr<CameraController> controller;

    // Helper to compare vectors with tolerance
    bool vec3Equal(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.0001f) {
        return glm::all(glm::lessThan(glm::abs(a - b), glm::vec3(epsilon)));
    }
};

// Test camera state management
TEST_F(CameraControllerTests, StateManagement) {
    // Get initial state
    CameraState state = controller->getState();
    
    // Modify state
    state.position = glm::vec3(10.0f, 5.0f, 3.0f);
    state.target = glm::vec3(0.0f);
    state.fieldOfView = 60.0f;
    
    // Set state without animation
    controller->setState(state, false);
    
    // Verify state was updated
    CameraState newState = controller->getState();
    EXPECT_TRUE(vec3Equal(newState.position, state.position));
    EXPECT_TRUE(vec3Equal(newState.target, state.target));
    EXPECT_FLOAT_EQ(newState.fieldOfView, state.fieldOfView);
}

// Test constraint application
TEST_F(CameraControllerTests, Constraints) {
    CameraConstraints constraints;
    constraints.minDistance = 5.0f;
    constraints.maxDistance = 20.0f;
    constraints.minFieldOfView = 30.0f;
    constraints.maxFieldOfView = 90.0f;
    
    controller->setConstraints(constraints);
    
    // Test distance constraints
    CameraState state = controller->getState();
    state.position = glm::vec3(0.0f, 0.0f, 2.0f);  // Too close
    controller->setState(state, false);
    
    state = controller->getState();
    float distance = glm::length(state.position - state.target);
    EXPECT_GE(distance, constraints.minDistance);
    
    // Test field of view constraints
    state.fieldOfView = 15.0f;  // Below minimum
    controller->setState(state, false);
    EXPECT_GE(controller->getState().fieldOfView, constraints.minFieldOfView);
    
    state.fieldOfView = 120.0f;  // Above maximum
    controller->setState(state, false);
    EXPECT_LE(controller->getState().fieldOfView, constraints.maxFieldOfView);
}

// Test standard views
TEST_F(CameraControllerTests, StandardViews) {
    // Test front view
    controller->setStandardView(ViewportView::Front, false);
    CameraState state = controller->getState();
    EXPECT_GT(state.position.z, 0.0f);
    EXPECT_FLOAT_EQ(state.position.x, 0.0f);
    EXPECT_FLOAT_EQ(state.position.y, 0.0f);
    
    // Test top view
    controller->setStandardView(ViewportView::Top, false);
    state = controller->getState();
    EXPECT_GT(state.position.y, 0.0f);
    EXPECT_FLOAT_EQ(state.position.x, 0.0f);
    EXPECT_FLOAT_EQ(state.position.z, 0.0f);
    
    // Test right view
    controller->setStandardView(ViewportView::Right, false);
    state = controller->getState();
    EXPECT_GT(state.position.x, 0.0f);
    EXPECT_FLOAT_EQ(state.position.y, 0.0f);
    EXPECT_FLOAT_EQ(state.position.z, 0.0f);
}

// Test camera operations
TEST_F(CameraControllerTests, CameraOperations) {
    // Test orbit
    CameraState initialState = controller->getState();
    controller->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, GLFW_MOD_ALT);
    controller->handleMouseMove(100.0, 0.0);  // Orbit horizontally
    
    CameraState orbitedState = controller->getState();
    EXPECT_NE(orbitedState.position, initialState.position);
    EXPECT_EQ(glm::length(orbitedState.position - orbitedState.target),
              glm::length(initialState.position - initialState.target));
    
    // Test pan
    initialState = controller->getState();
    controller->handleMouseButton(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0);
    controller->handleMouseMove(100.0, 0.0);  // Pan horizontally
    
    CameraState pannedState = controller->getState();
    EXPECT_NE(pannedState.target, initialState.target);
    EXPECT_EQ(glm::length(pannedState.position - pannedState.target),
              glm::length(initialState.position - initialState.target));
    
    // Test zoom
    initialState = controller->getState();
    controller->handleScroll(0.0, 1.0);  // Zoom in
    
    CameraState zoomedState = controller->getState();
    EXPECT_LT(glm::length(zoomedState.position - zoomedState.target),
              glm::length(initialState.position - initialState.target));
}

// Test frame region
TEST_F(CameraControllerTests, FrameRegion) {
    glm::vec3 min(-5.0f, -5.0f, -5.0f);
    glm::vec3 max(5.0f, 5.0f, 5.0f);
    
    controller->frameRegion(min, max, false);
    CameraState state = controller->getState();
    
    // Target should be at center of region
    glm::vec3 expectedTarget = (min + max) * 0.5f;
    EXPECT_TRUE(vec3Equal(state.target, expectedTarget));
    
    // Distance should be sufficient to view entire region
    float distance = glm::length(state.position - state.target);
    float maxDimension = glm::length(max - min);
    EXPECT_GT(distance, maxDimension * 0.5f);
}

// Test smooth transitions
TEST_F(CameraControllerTests, SmoothTransitions) {
    CameraSettings settings = controller->getSettings();
    settings.smoothTransitions = true;
    settings.transitionDuration = 0.5f;
    controller->setSettings(settings);
    
    // Start transition
    CameraState targetState = controller->getState();
    targetState.position = glm::vec3(10.0f, 5.0f, 3.0f);
    controller->setState(targetState, true);
    
    // Check intermediate state
    controller->update(0.25f);  // Half-way through transition
    CameraState midState = controller->getState();
    
    // Should be between initial and target positions
    EXPECT_NE(midState.position, targetState.position);
    
    // Complete transition
    controller->update(0.25f);
    CameraState finalState = controller->getState();
    EXPECT_TRUE(vec3Equal(finalState.position, targetState.position));
}

// Test input handling
TEST_F(CameraControllerTests, InputHandling) {
    // Test modifier key tracking
    controller->handleKeyboard(GLFW_KEY_LEFT_ALT, GLFW_PRESS, GLFW_MOD_ALT);
    controller->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, GLFW_MOD_ALT);
    
    CameraState initialState = controller->getState();
    controller->handleMouseMove(100.0, 0.0);
    
    // Should orbit when Alt is held
    CameraState newState = controller->getState();
    EXPECT_NE(newState.position, initialState.position);
    
    // Release modifier
    controller->handleKeyboard(GLFW_KEY_LEFT_ALT, GLFW_RELEASE, 0);
    initialState = controller->getState();
    controller->handleMouseMove(100.0, 0.0);
    
    // Should rotate when Alt is not held
    newState = controller->getState();
    EXPECT_NE(newState.rotation, initialState.rotation);
}

// Test settings
TEST_F(CameraControllerTests, Settings) {
    CameraSettings settings = controller->getSettings();
    settings.invertY = true;
    settings.orbitSpeed = 0.01f;
    controller->setSettings(settings);
    
    // Test inverted Y
    CameraState initialState = controller->getState();
    controller->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, GLFW_MOD_ALT);
    controller->handleMouseMove(0.0, 100.0);
    
    CameraState newState = controller->getState();
    glm::vec3 initialUp = glm::normalize(initialState.position - initialState.target);
    glm::vec3 newUp = glm::normalize(newState.position - newState.target);
    
    // Movement should be inverted
    EXPECT_NE(initialUp, newUp);
}

// Test focus operations
TEST_F(CameraControllerTests, FocusOperations) {
    glm::vec3 focusPoint(10.0f, 5.0f, 3.0f);
    float focusDistance = 20.0f;
    
    controller->focusOn(focusPoint, focusDistance, false);
    CameraState state = controller->getState();
    
    // Target should be at focus point
    EXPECT_TRUE(vec3Equal(state.target, focusPoint));
    
    // Distance should match requested
    EXPECT_FLOAT_EQ(glm::length(state.position - state.target), focusDistance);
}
