#include "ui/MainWindow.h"
#include "imgui.h"
#include "imgui_internal.h"  // Must be included after imgui.h
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// ImGui viewport flags
#ifndef ImGuiConfigFlags_ViewportsEnable
#define ImGuiConfigFlags_ViewportsEnable     (1 << 10)  // Enable Multi-Viewport / Platform Windows
#endif

#ifndef ImGuiConfigFlags_DockingEnable
#define ImGuiConfigFlags_DockingEnable       (1 << 6)   // Enable Docking
#endif

namespace RebelCAD {
namespace UI {

MainWindow::MainWindow(const std::string& title, int width, int height)
    : m_Title(title)
    , m_Width(width)
    , m_Height(height)
    , m_Initialized(false)
    , m_ShouldClose(false)
    , m_GraphicsSystem(std::make_shared<Graphics::GraphicsSystem>())
    , m_ToolbarManager(std::make_unique<ToolbarManager>(this))
{
}

MainWindow::~MainWindow() {
    if (m_Initialized) {
        shutdown();
    }
}

void MainWindow::initialize() {
    if (m_Initialized) {
        REBEL_LOG_WARNING("MainWindow already initialized");
        return;
    }

    try {
        // Initialize monitor manager
        MonitorManager::getInstance().initialize();
        setupMonitorCallback();
        // Initialize graphics system
        m_GraphicsSystem->initialize(
            Graphics::GraphicsAPI::OpenGL,
            Graphics::WindowProperties(m_Title, m_Width, m_Height)
        );
        
        // Setup Dear ImGui
        setupImGui();
        
        // Setup default panels
        setupDefaultPanels();
        
        // Setup context menus
        setupContextMenus();
        
        m_Initialized = true;
        REBEL_LOG_INFO("MainWindow initialized successfully");
    }
    catch (const std::exception& e) {
        REBEL_LOG_ERROR("Failed to initialize MainWindow: {}", e.what());
        throw;
    }
}

void MainWindow::setupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.ConfigViewportsNoAutoMerge = true;                    // Prevent viewports from auto-merging
    io.ConfigViewportsNoTaskBarIcon = false;                 // Show taskbar icons for each window
    io.ConfigWindowsMoveFromTitleBarOnly = true;             // More CAD-like window behavior
    io.ConfigWindowsResizeFromEdges = true;                  // Enable resizing from edges
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    // Setup Platform/Renderer backends
    GLFWwindow* window = static_cast<GLFWwindow*>(m_GraphicsSystem->getWindow());
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void MainWindow::render() {
    if (!m_Initialized) {
        REBEL_LOG_ERROR("Cannot render uninitialized MainWindow");
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render main menu bar
    renderMainMenuBar();

    // Render toolbars
    m_ToolbarManager->render();

    // Handle context menus
    handleContextMenus();

    // Render all docked panels through DockingManager
    DockingManager::getInstance().renderPanels();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    m_GraphicsSystem->present();
}

void MainWindow::setupDefaultPanels() {
    // Create and register default panels
    class DefaultPanel : public Panel {
    public:
        DefaultPanel(const std::string& title) : Panel(title) {}
        void render() override {
            ImGui::Text("This is the %s panel", getTitle().c_str());
            // Add default content here
        }
    };

    // Create default panels
    auto propertiesPanel = std::make_shared<DefaultPanel>("Properties");
    auto featureTreePanel = std::make_shared<DefaultPanel>("Feature Tree");
    auto consolePanel = std::make_shared<DefaultPanel>("Console");
    auto layerManagerPanel = std::make_shared<DefaultPanel>("Layer Manager");
    auto toolboxPanel = std::make_shared<DefaultPanel>("Toolbox");

    // Register panels with DockingManager
    auto& dockManager = DockingManager::getInstance();
    dockManager.registerPanel("properties", propertiesPanel);
    dockManager.registerPanel("featureTree", featureTreePanel);
    dockManager.registerPanel("console", consolePanel);
    dockManager.registerPanel("layerManager", layerManagerPanel);
    dockManager.registerPanel("toolbox", toolboxPanel);

    // Apply default layout
    dockManager.resetToDefaultLayout();
}

void MainWindow::registerPanel(const std::string& name, std::shared_ptr<Panel> panel) {
    DockingManager::getInstance().registerPanel(name, panel);
}

void MainWindow::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project", "Ctrl+N")) {}
            if (ImGui::MenuItem("New Part", "Ctrl+Shift+N")) {}
            if (ImGui::MenuItem("New Assembly", "Ctrl+Alt+N")) {}
            ImGui::Separator();
            
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {}
            if (ImGui::BeginMenu("Open Recent")) {
                // TODO: Implement recent files list
                if (ImGui::MenuItem("(Empty)")) {}
                ImGui::EndMenu();
            }
            ImGui::Separator();
            
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {}
            if (ImGui::MenuItem("Save All", "Ctrl+Alt+S")) {}
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Import")) {
                if (ImGui::MenuItem("STEP File...")) {}
                if (ImGui::MenuItem("IGES File...")) {}
                if (ImGui::MenuItem("STL File...")) {}
                if (ImGui::MenuItem("DXF File...")) {}
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Export")) {
                if (ImGui::MenuItem("STEP File...")) {}
                if (ImGui::MenuItem("IGES File...")) {}
                if (ImGui::MenuItem("STL File...")) {}
                if (ImGui::MenuItem("DXF File...")) {}
                ImGui::EndMenu();
            }
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                m_ShouldClose = true;
            }
            ImGui::EndMenu();
        }

        // Edit Menu
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
            if (ImGui::MenuItem("Delete", "Del")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {}
            if (ImGui::MenuItem("Deselect All", "Ctrl+D")) {}
            ImGui::EndMenu();
        }

        // View Menu
        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Workspaces")) {
                if (ImGui::MenuItem("Default")) {
                    DockingManager::getInstance().resetToDefaultLayout();
                }
                if (ImGui::MenuItem("Modeling")) {}
                if (ImGui::MenuItem("Assembly")) {}
                if (ImGui::MenuItem("Drafting")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Save Current Layout...")) {}
                if (ImGui::MenuItem("Reset Layout")) {
                    DockingManager::getInstance().resetToDefaultLayout();
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Panels")) {
                auto& dockManager = DockingManager::getInstance();
                for (const auto& name : dockManager.getPanelNames()) {
                    bool isVisible = dockManager.isPanelVisible(name);
                    if (ImGui::MenuItem(name.c_str(), nullptr, &isVisible)) {
                        dockManager.showPanel(name, isVisible);
                    }
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Toolbars")) {
                if (ImGui::MenuItem("Standard")) {}
                if (ImGui::MenuItem("Sketch")) {}
                if (ImGui::MenuItem("Features")) {}
                if (ImGui::MenuItem("Assembly")) {}
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Display")) {
                if (ImGui::MenuItem("Wireframe")) {}
                if (ImGui::MenuItem("Hidden Line")) {}
                if (ImGui::MenuItem("Shaded")) {}
                if (ImGui::MenuItem("Realistic")) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Monitors")) {
                auto& monitorManager = MonitorManager::getInstance();
                auto monitors = monitorManager.getMonitors();
                
                for (const auto& monitor : monitors) {
                    if (ImGui::MenuItem(monitor.name.c_str(), nullptr, monitor.isPrimary)) {
                        moveToMonitor(monitor);
                    }
                }
                
                ImGui::Separator();
                
                if (ImGui::BeginMenu("Window Mode")) {
                    if (ImGui::MenuItem("Centered")) {
                        auto* currentMonitor = monitorManager.getMonitorAtPosition(
                            m_GraphicsSystem->getWindowPosX(),
                            m_GraphicsSystem->getWindowPosY()
                        );
                        if (currentMonitor) {
                            moveToMonitor(*currentMonitor, true);
                        }
                    }
                    if (ImGui::MenuItem("Maximized")) {
                        auto* currentMonitor = monitorManager.getMonitorAtPosition(
                            m_GraphicsSystem->getWindowPosX(),
                            m_GraphicsSystem->getWindowPosY()
                        );
                        if (currentMonitor) {
                            maximizeOnMonitor(*currentMonitor);
                        }
                    }
                    ImGui::EndMenu();
                }
                
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        // Sketch Menu
        if (ImGui::BeginMenu("Sketch")) {
            if (ImGui::MenuItem("New Sketch")) {}
            if (ImGui::MenuItem("Edit Sketch")) {}
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Line", "L")) {}
                if (ImGui::MenuItem("Circle", "C")) {}
                if (ImGui::MenuItem("Arc", "A")) {}
                if (ImGui::MenuItem("Rectangle", "R")) {}
                if (ImGui::MenuItem("Polygon", "P")) {}
                if (ImGui::MenuItem("Spline", "S")) {}
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Constraints")) {
                if (ImGui::MenuItem("Horizontal")) {}
                if (ImGui::MenuItem("Vertical")) {}
                if (ImGui::MenuItem("Parallel")) {}
                if (ImGui::MenuItem("Perpendicular")) {}
                if (ImGui::MenuItem("Coincident")) {}
                if (ImGui::MenuItem("Concentric")) {}
                if (ImGui::MenuItem("Equal")) {}
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        // Model Menu
        if (ImGui::BeginMenu("Model")) {
            if (ImGui::BeginMenu("Features")) {
                if (ImGui::MenuItem("Extrude")) {}
                if (ImGui::MenuItem("Revolve")) {}
                if (ImGui::MenuItem("Sweep")) {}
                if (ImGui::MenuItem("Loft")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Shell")) {}
                if (ImGui::MenuItem("Fillet")) {}
                if (ImGui::MenuItem("Chamfer")) {}
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Surface")) {
                if (ImGui::MenuItem("Create Plane")) {}
                if (ImGui::MenuItem("Extend Surface")) {}
                if (ImGui::MenuItem("Trim Surface")) {}
                if (ImGui::MenuItem("Offset Surface")) {}
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Pattern")) {
                if (ImGui::MenuItem("Linear Pattern")) {}
                if (ImGui::MenuItem("Circular Pattern")) {}
                if (ImGui::MenuItem("Mirror")) {}
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        // Assembly Menu
        if (ImGui::BeginMenu("Assembly")) {
            if (ImGui::MenuItem("Insert Component")) {}
            if (ImGui::MenuItem("Replace Component")) {}
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Constraints")) {
                if (ImGui::MenuItem("Mate")) {}
                if (ImGui::MenuItem("Align")) {}
                if (ImGui::MenuItem("Angle")) {}
                if (ImGui::MenuItem("Distance")) {}
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Create Pattern")) {}
            if (ImGui::MenuItem("Create Mirror")) {}
            ImGui::Separator();
            
            if (ImGui::MenuItem("Exploded View")) {}
            
            ImGui::EndMenu();
        }

        // Analysis Menu
        if (ImGui::BeginMenu("Analysis")) {
            if (ImGui::MenuItem("Mass Properties")) {}
            if (ImGui::MenuItem("Interference Check")) {}
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Simulation")) {
                if (ImGui::MenuItem("Static Analysis")) {}
                if (ImGui::MenuItem("Dynamic Analysis")) {}
                if (ImGui::MenuItem("Thermal Analysis")) {}
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        // Tools Menu
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Options...")) {}
            if (ImGui::MenuItem("Customize...")) {}
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Macros")) {
                if (ImGui::MenuItem("Record Macro")) {}
                if (ImGui::MenuItem("Stop Recording")) {}
                if (ImGui::MenuItem("Edit Macros...")) {}
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Plugins...")) {}
            
            ImGui::EndMenu();
        }

        // Help Menu
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {}
            if (ImGui::MenuItem("Tutorials")) {}
            if (ImGui::MenuItem("Sample Models")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Check for Updates...")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("About RebelCAD")) {}
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void MainWindow::moveToMonitor(const MonitorInfo& monitor, bool centered) {
    if (!m_Initialized) return;
    
    GLFWwindow* window = static_cast<GLFWwindow*>(m_GraphicsSystem->getWindow());
    MonitorManager::getInstance().moveWindowToMonitor(window, monitor, centered);
}

void MainWindow::maximizeOnMonitor(const MonitorInfo& monitor) {
    if (!m_Initialized) return;
    
    GLFWwindow* window = static_cast<GLFWwindow*>(m_GraphicsSystem->getWindow());
    MonitorManager::getInstance().maximizeWindowOnMonitor(window, monitor);
}

void MainWindow::setupMonitorCallback() {
    MonitorManager::getInstance().setMonitorChangeCallback(
        [this]() { handleMonitorChange(); }
    );
}

void MainWindow::handleMonitorChange() {
    // Ensure window is on a valid monitor after monitor configuration changes
    auto* currentMonitor = MonitorManager::getInstance().getMonitorAtPosition(
        m_GraphicsSystem->getWindowPosX(),
        m_GraphicsSystem->getWindowPosY()
    );
    
    if (!currentMonitor) {
        // Window is not on any monitor, move it to primary
        auto* primary = MonitorManager::getInstance().getPrimaryMonitor();
        if (primary) {
            moveToMonitor(*primary, true);
        }
    }
}

void MainWindow::shutdown() {
    if (!m_Initialized) {
        return;
    }

    // Cleanup ImGui
    cleanupImGui();
    
    // Shutdown graphics system
    m_GraphicsSystem->shutdown();
    
    // Reset toolbar manager
    m_ToolbarManager.reset();
    
    m_Initialized = false;
    REBEL_LOG_INFO("MainWindow shut down successfully");
}

void MainWindow::cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void MainWindow::setupContextMenus() {
    auto& contextManager = ContextMenuManager::getInstance();
    
    // Setup sketch context menu
    std::vector<ContextMenuItem> sketchItems = {
        ContextMenuItem("Edit Sketch", []() { /* TODO: Implement */ }),
        ContextMenuItem("Delete Sketch", []() { /* TODO: Implement */ }),
        ContextMenuItem::Separator(),
        ContextMenuItem("Add Constraint", []() { /* TODO: Implement */ }),
        ContextMenuItem("Show/Hide", []() { /* TODO: Implement */ })
    };
    contextManager.registerContextMenu("sketch", sketchItems);
    
    // Setup 3D model context menu
    std::vector<ContextMenuItem> modelItems = {
        ContextMenuItem("Edit Feature", []() { /* TODO: Implement */ }),
        ContextMenuItem("Suppress Feature", []() { /* TODO: Implement */ }),
        ContextMenuItem::Separator(),
        ContextMenuItem("Hide", []() { /* TODO: Implement */ }),
        ContextMenuItem("Show", []() { /* TODO: Implement */ }),
        ContextMenuItem::Separator(),
        ContextMenuItem("Properties", []() { /* TODO: Implement */ })
    };
    contextManager.registerContextMenu("model", modelItems);
    
    // Setup panel context menu
    std::vector<ContextMenuItem> panelItems = {
        ContextMenuItem("Float", []() { /* TODO: Implement */ }),
        ContextMenuItem("Dock", []() { /* TODO: Implement */ }),
        ContextMenuItem::Separator(),
        ContextMenuItem("Close", []() { /* TODO: Implement */ })
    };
    contextManager.registerContextMenu("panel", panelItems);
}

void MainWindow::handleContextMenus() {
    auto& contextManager = ContextMenuManager::getInstance();
    
    // Check for right-click in different areas and show appropriate context menu
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        // Get mouse position and check which area was clicked
        ImVec2 mousePos = ImGui::GetMousePos();
        
        // Example: Check if mouse is in sketch area
        // TODO: Implement proper area detection
        if (/* isInSketchArea(mousePos) */ false) {
            contextManager.showContextMenu("sketch");
        }
        // Example: Check if mouse is on a 3D model
        else if (/* isOnModel(mousePos) */ false) {
            contextManager.showContextMenu("model");
        }
        // Example: Check if mouse is on a panel
        else if (/* isOnPanel(mousePos) */ false) {
            contextManager.showContextMenu("panel");
        }
    }
}

void MainWindow::setSize(int width, int height) {
    m_Width = width;
    m_Height = height;
    if (m_Initialized) {
        m_GraphicsSystem->setWindowSize(width, height);
    }
}

void MainWindow::setTitle(const std::string& title) {
    m_Title = title;
    if (m_Initialized) {
        m_GraphicsSystem->setWindowTitle(title);
    }
}

bool MainWindow::shouldClose() const {
    return m_ShouldClose || (m_Initialized && m_GraphicsSystem->shouldClose());
}

} // namespace UI
} // namespace RebelCAD
