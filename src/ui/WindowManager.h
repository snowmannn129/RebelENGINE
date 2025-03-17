#pragma once

#include "ui/MainWindow.h"
#include <vector>
#include <memory>
#include <string>

namespace RebelCAD {
namespace UI {

class WindowManager {
public:
    // Singleton access
    static WindowManager& getInstance();

    // Delete copy constructor and assignment operator
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    // Window management functions
    std::shared_ptr<MainWindow> createWindow(const std::string& title = "RebelCAD", 
                                           int width = 1280, 
                                           int height = 720);
    void destroyWindow(std::shared_ptr<MainWindow> window);
    void destroyAllWindows();

    // Window queries
    std::vector<std::shared_ptr<MainWindow>> getWindows() const;
    std::shared_ptr<MainWindow> getActiveWindow() const;
    size_t getWindowCount() const;

    // Window updates
    void updateAll();
    void renderAll();

private:
    WindowManager() = default;  // Private constructor for singleton
    ~WindowManager();

    std::vector<std::shared_ptr<MainWindow>> m_Windows;
    std::shared_ptr<MainWindow> m_ActiveWindow;

    // Internal management
    void setActiveWindow(std::shared_ptr<MainWindow> window);
    void removeClosedWindows();
};

} // namespace UI
} // namespace RebelCAD
