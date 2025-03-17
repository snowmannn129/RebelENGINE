#include "ui/MonitorManager.h"

namespace RebelCAD {
namespace UI {

MonitorInfo::MonitorInfo(GLFWmonitor* monitor) : handle(monitor) {
    // Get monitor name
    name = glfwGetMonitorName(monitor);

    // Get monitor video mode
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    width = mode->width;
    height = mode->height;
    refreshRate = mode->refreshRate;

    // Get content scale (DPI)
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    dpiScale = xscale; // Use horizontal scale as reference

    // Get work area (screen space without taskbar/dock)
    glfwGetMonitorWorkarea(monitor, &workAreaX, &workAreaY, &workAreaWidth, &workAreaHeight);

    // Check if this is the primary monitor
    isPrimary = (monitor == glfwGetPrimaryMonitor());
}

void MonitorManager::initialize() {
    // Set up monitor callback
    glfwSetMonitorCallback(monitorCallback);
    
    // Initial monitor detection
    refreshMonitorList();
    
    REBEL_LOG_INFO("MonitorManager initialized with {} monitors", m_Monitors.size());
}

void MonitorManager::monitorCallback(GLFWmonitor* monitor, int event) {
    auto& instance = MonitorManager::getInstance();
    
    if (event == GLFW_CONNECTED) {
        REBEL_LOG_INFO("Monitor connected: {}", glfwGetMonitorName(monitor));
    } else if (event == GLFW_DISCONNECTED) {
        REBEL_LOG_INFO("Monitor disconnected: {}", glfwGetMonitorName(monitor));
    }
    
    // Refresh monitor list
    instance.refreshMonitorList();
    
    // Notify listeners
    if (instance.m_MonitorChangeCallback) {
        instance.m_MonitorChangeCallback();
    }
}

void MonitorManager::refreshMonitorList() {
    m_Monitors.clear();
    
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    
    for (int i = 0; i < count; i++) {
        m_Monitors.emplace_back(monitors[i]);
    }
}

std::vector<MonitorInfo> MonitorManager::getMonitors() const {
    return m_Monitors;
}

MonitorInfo* MonitorManager::getPrimaryMonitor() {
    for (auto& monitor : m_Monitors) {
        if (monitor.isPrimary) {
            return &monitor;
        }
    }
    return nullptr;
}

MonitorInfo* MonitorManager::getMonitorAtPosition(int x, int y) {
    for (auto& monitor : m_Monitors) {
        // Check if position is within monitor work area
        if (x >= monitor.workAreaX && 
            x < monitor.workAreaX + monitor.workAreaWidth &&
            y >= monitor.workAreaY && 
            y < monitor.workAreaY + monitor.workAreaHeight) {
            return &monitor;
        }
    }
    return nullptr;
}

void MonitorManager::setMonitorChangeCallback(std::function<void()> callback) {
    m_MonitorChangeCallback = callback;
}

void MonitorManager::moveWindowToMonitor(GLFWwindow* window, const MonitorInfo& monitor, bool centered) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    
    int xpos, ypos;
    if (centered) {
        xpos = monitor.workAreaX + (monitor.workAreaWidth - windowWidth) / 2;
        ypos = monitor.workAreaY + (monitor.workAreaHeight - windowHeight) / 2;
    } else {
        xpos = monitor.workAreaX;
        ypos = monitor.workAreaY;
    }
    
    glfwSetWindowPos(window, xpos, ypos);
}

void MonitorManager::maximizeWindowOnMonitor(GLFWwindow* window, const MonitorInfo& monitor) {
    // First move window to monitor
    moveWindowToMonitor(window, monitor, false);
    
    // Then maximize it
    glfwMaximizeWindow(window);
}

} // namespace UI
} // namespace RebelCAD
