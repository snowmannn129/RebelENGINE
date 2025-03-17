#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <GLFW/glfw3.h>
#include "core/Log.h"

namespace RebelCAD {
namespace UI {

// Forward declarations
namespace Tests {
class MonitorManagerTest;
}

struct MonitorInfo {
    GLFWmonitor* handle;
    std::string name;
    int width;
    int height;
    int refreshRate;
    float dpiScale;
    int workAreaX;
    int workAreaY;
    int workAreaWidth;
    int workAreaHeight;
    bool isPrimary;

    MonitorInfo(GLFWmonitor* monitor);
};

class MonitorManager {
public:
    static MonitorManager& getInstance() {
        static MonitorManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    MonitorManager(const MonitorManager&) = delete;
    MonitorManager& operator=(const MonitorManager&) = delete;

    // Initialize monitor detection
    void initialize();

    // Get monitor information
    std::vector<MonitorInfo> getMonitors() const;
    MonitorInfo* getPrimaryMonitor();
    MonitorInfo* getMonitorAtPosition(int x, int y);
    
    // Monitor change callback
    void setMonitorChangeCallback(std::function<void()> callback);

    // Window positioning helpers
    void moveWindowToMonitor(GLFWwindow* window, const MonitorInfo& monitor, bool centered = true);
    void maximizeWindowOnMonitor(GLFWwindow* window, const MonitorInfo& monitor);

    // Test helper methods
    #ifdef TESTING
    void simulateMonitorEvent(GLFWmonitor* monitor, int event) {
        monitorCallback(monitor, event);
    }
    #endif

protected:
    // Make these protected for testing
    MonitorManager() = default;
    ~MonitorManager() = default;

    // GLFW monitor callback - protected for testing
    static void monitorCallback(GLFWmonitor* monitor, int event);
    void refreshMonitorList();

private:
    std::vector<MonitorInfo> m_Monitors;
    std::function<void()> m_MonitorChangeCallback;

    // Friend the test class to allow access to protected members
    friend class Tests::MonitorManagerTest;
};

} // namespace UI
} // namespace RebelCAD
