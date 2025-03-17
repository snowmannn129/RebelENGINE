#pragma once

#include <string>
#include <memory>
#include <vector>
#include "graphics/GraphicsSystem.h"
#include "ui/DockingManager.h"
#include "ui/ToolbarManager.h"
#include "ui/ContextMenuManager.h"
#include "ui/MonitorManager.h"
#include "core/Log.h"

namespace RebelCAD {
namespace UI {

class MainWindow {
public:
    MainWindow(const std::string& title = "RebelCAD", int width = 1280, int height = 720);
    ~MainWindow();

    // Delete copy constructor and assignment operator
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    // Core window management functions
    void initialize();
    void render();
    void shutdown();
    bool shouldClose() const;

    // Panel management
    void setupDefaultPanels();
    void registerPanel(const std::string& name, std::shared_ptr<Panel> panel);
    
    // Window property accessors
    int getWidth() const { return m_Width; }
    int getHeight() const { return m_Height; }
    const std::string& getTitle() const { return m_Title; }
    
    // Window property mutators
    void setSize(int width, int height);
    void setTitle(const std::string& title);
    
    // Monitor management
    void moveToMonitor(const MonitorInfo& monitor, bool centered = true);
    void maximizeOnMonitor(const MonitorInfo& monitor);
    void handleMonitorChange();

private:
    // Monitor change callback
    void setupMonitorCallback();
    std::string m_Title;
    int m_Width;
    int m_Height;
    bool m_Initialized;
    bool m_ShouldClose;
    
    // Graphics system reference
    std::shared_ptr<Graphics::GraphicsSystem> m_GraphicsSystem;
    
    // UI Managers
    std::unique_ptr<ToolbarManager> m_ToolbarManager;
    
    // Context menu handling
    void setupContextMenus();
    void handleContextMenus();

    // ImGui setup and cleanup
    void setupImGui();
    void cleanupImGui();
    
    // Main menu bar rendering
    void renderMainMenuBar();
};

} // namespace UI
} // namespace RebelCAD
