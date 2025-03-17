#include "ui/InputManager.h"
#include "core/Log.h"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace RebelCAD {
namespace UI {

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

InputManager::InputManager()
    : m_Window(nullptr)
    , m_MouseX(0.0)
    , m_MouseY(0.0)
    , m_LastMouseX(0.0)
    , m_LastMouseY(0.0)
    , m_ScrollX(0.0)
    , m_ScrollY(0.0)
{
    setupEventHandlers();
}

InputManager::~InputManager() {
    shutdown();
}

void InputManager::initialize(GLFWwindow* window) {
    m_Window = window;

    // Set GLFW callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCharCallback(window, charCallback);

    // Store user pointer for callbacks
    glfwSetWindowUserPointer(window, this);

    REBEL_LOG_INFO("Input manager initialized");
}

void InputManager::shutdown() {
    if (m_Window) {
        // Remove callbacks
        glfwSetKeyCallback(m_Window, nullptr);
        glfwSetMouseButtonCallback(m_Window, nullptr);
        glfwSetCursorPosCallback(m_Window, nullptr);
        glfwSetScrollCallback(m_Window, nullptr);
        glfwSetCharCallback(m_Window, nullptr);
        
        m_Window = nullptr;
    }

    // Clear all state
    m_KeyStates.clear();
    m_MouseButtonStates.clear();
    m_KeyMappings.clear();
    
    while (!m_KeyEvents.empty()) m_KeyEvents.pop();
    while (!m_MouseButtonEvents.empty()) m_MouseButtonEvents.pop();
    while (!m_MouseMoveEvents.empty()) m_MouseMoveEvents.pop();
    while (!m_ScrollEvents.empty()) m_ScrollEvents.pop();
    while (!m_CharEvents.empty()) m_CharEvents.pop();

    REBEL_LOG_INFO("Input manager shut down");
}

void InputManager::update() {
    // Store previous mouse position
    m_LastMouseX = m_MouseX;
    m_LastMouseY = m_MouseY;

    // Process queued events
    processInputEvents();

    // Update input states
    updateInputStates();

    // Clear single-frame states
    clearInputStates();
}

void InputManager::processInputEvents() {
    // Process keyboard events
    while (!m_KeyEvents.empty()) {
        const auto& event = m_KeyEvents.front();
        
        // Update key state
        auto& state = m_KeyStates[event.key];
        state.pressed = (event.action == GLFW_PRESS);
        state.released = (event.action == GLFW_RELEASE);
        state.held = (event.action == GLFW_REPEAT || 
                     (event.action == GLFW_PRESS && state.held));

        // Check for mapped actions
        auto it = m_KeyMappings.find(event.key);
        if (it != m_KeyMappings.end() && event.action == GLFW_PRESS) {
            it->second.callback();
        }

        // Publish event through EventBus
        auto& eventBus = Core::EventBus::getInstance();
        eventBus.publish(event);

        m_KeyEvents.pop();
    }

    // Process mouse button events
    while (!m_MouseButtonEvents.empty()) {
        const auto& event = m_MouseButtonEvents.front();
        
        // Update button state
        auto& state = m_MouseButtonStates[event.button];
        state.pressed = (event.action == GLFW_PRESS);
        state.released = (event.action == GLFW_RELEASE);
        state.held = (event.action == GLFW_REPEAT || 
                     (event.action == GLFW_PRESS && state.held));

        // Publish event through EventBus
        auto& eventBus = Core::EventBus::getInstance();
        eventBus.publish(event);

        m_MouseButtonEvents.pop();
    }

    // Process mouse move events
    while (!m_MouseMoveEvents.empty()) {
        const auto& event = m_MouseMoveEvents.front();
        
        // Update mouse position
        m_MouseX = event.x;
        m_MouseY = event.y;

        // Publish event through EventBus
        auto& eventBus = Core::EventBus::getInstance();
        eventBus.publish(event);

        m_MouseMoveEvents.pop();
    }

    // Process scroll events
    while (!m_ScrollEvents.empty()) {
        const auto& event = m_ScrollEvents.front();
        
        // Update scroll values
        m_ScrollX += event.xoffset;
        m_ScrollY += event.yoffset;

        // Publish event through EventBus
        auto& eventBus = Core::EventBus::getInstance();
        eventBus.publish(event);

        m_ScrollEvents.pop();
    }

    // Process character input events
    while (!m_CharEvents.empty()) {
        const auto& event = m_CharEvents.front();
        
        // Publish event through EventBus
        auto& eventBus = Core::EventBus::getInstance();
        eventBus.publish(event);

        m_CharEvents.pop();
    }
}

void InputManager::updateInputStates() {
    // Update held states for keys
    for (auto& [key, state] : m_KeyStates) {
        if (state.pressed) {
            state.held = true;
        }
        else if (state.released) {
            state.held = false;
        }
    }

    // Update held states for mouse buttons
    for (auto& [button, state] : m_MouseButtonStates) {
        if (state.pressed) {
            state.held = true;
        }
        else if (state.released) {
            state.held = false;
        }
    }
}

void InputManager::clearInputStates() {
    // Clear single-frame states (pressed/released) but maintain held state
    for (auto& [key, state] : m_KeyStates) {
        state.pressed = false;
        state.released = false;
    }

    for (auto& [button, state] : m_MouseButtonStates) {
        state.pressed = false;
        state.released = false;
    }

    // Reset scroll values
    m_ScrollX = 0.0;
    m_ScrollY = 0.0;
}

bool InputManager::isKeyPressed(int key) const {
    auto it = m_KeyStates.find(key);
    return it != m_KeyStates.end() && it->second.pressed;
}

bool InputManager::isKeyReleased(int key) const {
    auto it = m_KeyStates.find(key);
    return it != m_KeyStates.end() && it->second.released;
}

bool InputManager::isKeyHeld(int key) const {
    auto it = m_KeyStates.find(key);
    return it != m_KeyStates.end() && it->second.held;
}

bool InputManager::isMouseButtonPressed(int button) const {
    auto it = m_MouseButtonStates.find(button);
    return it != m_MouseButtonStates.end() && it->second.pressed;
}

bool InputManager::isMouseButtonReleased(int button) const {
    auto it = m_MouseButtonStates.find(button);
    return it != m_MouseButtonStates.end() && it->second.released;
}

bool InputManager::isMouseButtonHeld(int button) const {
    auto it = m_MouseButtonStates.find(button);
    return it != m_MouseButtonStates.end() && it->second.held;
}

void InputManager::getMousePosition(double& x, double& y) const {
    x = m_MouseX;
    y = m_MouseY;
}

void InputManager::getMouseDelta(double& dx, double& dy) const {
    dx = m_MouseX - m_LastMouseX;
    dy = m_MouseY - m_LastMouseY;
}

double InputManager::getMouseScrollDelta() const {
    return m_ScrollY;  // Most common use case is vertical scroll
}

void InputManager::setFocusedWidget(const std::string& widgetId) {
    if (m_FocusedWidget != widgetId) {
        m_FocusedWidget = widgetId;
        
        // Notify through EventBus
        auto& eventBus = Core::EventBus::getInstance();
        struct FocusChangedEvent {
            std::string widgetId;
        };
        eventBus.publish(FocusChangedEvent{widgetId});
    }
}

std::string InputManager::getFocusedWidget() const {
    return m_FocusedWidget;
}

bool InputManager::hasFocus(const std::string& widgetId) const {
    return m_FocusedWidget == widgetId;
}

void InputManager::mapKeyToAction(int key, const std::string& action, InputCallback callback) {
    m_KeyMappings[key] = ActionMapping{action, callback};
    REBEL_LOG_DEBUG("Mapped key {} to action: {}", key, action);
}

void InputManager::unmapKey(int key) {
    auto it = m_KeyMappings.find(key);
    if (it != m_KeyMappings.end()) {
        REBEL_LOG_DEBUG("Unmapped key {} from action: {}", key, it->second.action);
        m_KeyMappings.erase(it);
    }
}

void InputManager::triggerAction(const std::string& action) {
    // Find and trigger all callbacks mapped to this action
    for (const auto& [key, mapping] : m_KeyMappings) {
        if (mapping.action == action) {
            mapping.callback();
        }
    }
}

void InputManager::setupEventHandlers() {
    auto& eventBus = Core::EventBus::getInstance();
    
    // Subscribe to any relevant events
    // Currently no subscriptions needed, but could add them here
}

// Static GLFW callback implementations
void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->m_KeyEvents.push(KeyEvent{key, scancode, action, mods});
    }
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->m_MouseButtonEvents.push(MouseButtonEvent{button, action, mods});
    }
}

void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        double dx = xpos - manager->m_MouseX;
        double dy = ypos - manager->m_MouseY;
        manager->m_MouseMoveEvents.push(MouseMoveEvent{xpos, ypos, dx, dy});
    }
}

void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->m_ScrollEvents.push(ScrollEvent{xoffset, yoffset});
    }
}

void InputManager::charCallback(GLFWwindow* window, unsigned int c) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->m_CharEvents.push(CharEvent{c});
    }
}

} // namespace UI
} // namespace RebelCAD
