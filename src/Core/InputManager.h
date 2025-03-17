#pragma once

#include "core/EventBus.h"
#include "imgui.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <queue>
#include <GLFW/glfw3.h>

struct GLFWwindow;

namespace RebelCAD {
namespace UI {

/**
 * @brief Manages input handling and event dispatching for the UI system
 * 
 * Handles:
 * - Keyboard input
 * - Mouse input
 * - Touch input
 * - Input focus
 * - Input event propagation
 */
class InputManager {
public:
    // Input event types
    enum class InputEventType {
        MouseMove,
        MouseButton,
        KeyPress,
        KeyRelease,
        Char,
        Scroll
    };

    // Input event data structures
    struct MouseData {
        int button;
        int action;
        int mods;
    };

    struct KeyData {
        int code;
        int mods;
    };

    struct InputEvent {
        InputEventType type;
        ImVec2 mousePos;      // For mouse events
        MouseData mouseButton; // For mouse button events
        KeyData key;          // For keyboard events
        unsigned int charCode;  // For character input
        ImVec2 scrollOffset;  // For scroll events
    };

    static InputManager& getInstance();

    // Delete copy constructor and assignment
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    /**
     * @brief Initialize the input manager
     * @param window GLFW window to handle input for
     */
    void initialize(GLFWwindow* window);

    /**
     * @brief Update input state and process events
     */
    void update();

    /**
     * @brief Clean up input manager resources
     */
    void shutdown();

    // Input state queries
    bool isKeyPressed(int key) const;
    bool isKeyReleased(int key) const;
    bool isKeyHeld(int key) const;
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    bool isMouseButtonHeld(int button) const;
    void getMousePosition(double& x, double& y) const;
    void getMouseDelta(double& dx, double& dy) const;
    double getMouseScrollDelta() const;

    // Focus management
    void setFocusedWidget(const std::string& widgetId);
    std::string getFocusedWidget() const;
    bool hasFocus(const std::string& widgetId) const;

    // Input mapping
    using InputCallback = std::function<void()>;
    void mapKeyToAction(int key, const std::string& action, InputCallback callback);
    void unmapKey(int key);
    void triggerAction(const std::string& action);

protected:
    // GLFW callbacks - protected for testing
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void charCallback(GLFWwindow* window, unsigned int c);

    // Friend test class
    friend class InputManagerTest;

private:
    InputManager();
    ~InputManager();

    // Event definitions
    struct KeyEvent {
        int key;
        int scancode;
        int action;
        int mods;
    };

    struct MouseButtonEvent {
        int button;
        int action;
        int mods;
    };

    struct MouseMoveEvent {
        double x;
        double y;
        double dx;
        double dy;
    };

    struct ScrollEvent {
        double xoffset;
        double yoffset;
    };

    struct CharEvent {
        unsigned int codepoint;
    };

    // Input state tracking
    struct InputState {
        bool pressed;
        bool held;
        bool released;
    };

    GLFWwindow* m_Window;
    std::unordered_map<int, InputState> m_KeyStates;
    std::unordered_map<int, InputState> m_MouseButtonStates;
    double m_MouseX, m_MouseY;
    double m_LastMouseX, m_LastMouseY;
    double m_ScrollX, m_ScrollY;
    std::string m_FocusedWidget;

    // Input mapping
    struct ActionMapping {
        std::string action;
        InputCallback callback;
    };
    std::unordered_map<int, ActionMapping> m_KeyMappings;

    // Event queues
    std::queue<KeyEvent> m_KeyEvents;
    std::queue<MouseButtonEvent> m_MouseButtonEvents;
    std::queue<MouseMoveEvent> m_MouseMoveEvents;
    std::queue<ScrollEvent> m_ScrollEvents;
    std::queue<CharEvent> m_CharEvents;

    // Event subscriptions
    size_t m_InputEventSubscription;

    void setupEventHandlers();
    void processInputEvents();
    void updateInputStates();
    void clearInputStates();
};

} // namespace UI
} // namespace RebelCAD
