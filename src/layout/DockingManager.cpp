#include "ui/DockingManager.h"
#include "core/Log.h"
#include "imgui_internal.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace RebelCAD {
namespace UI {

DockingManager& DockingManager::getInstance() {
    static DockingManager instance;
    return instance;
}

DockingManager::DockingManager()
    : m_DockspaceID(0)
    , m_DockspaceInitialized(false)
{
    setupEventHandlers();
}

DockingManager::~DockingManager() {
    // Unsubscribe from events
    auto& eventBus = Core::EventBus::getInstance();
    eventBus.unsubscribe(m_LayoutChangedSubscription);
}

void DockingManager::setupEventHandlers() {
    auto& eventBus = Core::EventBus::getInstance();
    
    // Subscribe to layout change events
    m_LayoutChangedSubscription = eventBus.subscribe<LayoutChangedEvent>(
        [this](const LayoutChangedEvent& event, const Core::EventMetadata&) {
            REBEL_LOG_INFO("Layout changed from: {}", event.source);
            // Could trigger layout save here
        }
    );
}

void DockingManager::registerPanel(const std::string& name, std::shared_ptr<Panel> panel) {
    if (m_Panels.find(name) != m_Panels.end()) {
        REBEL_LOG_WARNING("Panel '{}' already registered", name);
        return;
    }
    
    m_Panels[name] = panel;
    REBEL_LOG_INFO("Registered panel: {}", name);
}

void DockingManager::unregisterPanel(const std::string& name) {
    auto it = m_Panels.find(name);
    if (it != m_Panels.end()) {
        m_Panels.erase(it);
        REBEL_LOG_INFO("Unregistered panel: {}", name);
    }
}

std::vector<std::string> DockingManager::getPanelNames() const {
    std::vector<std::string> names;
    names.reserve(m_Panels.size());
    for (const auto& [name, _] : m_Panels) {
        names.push_back(name);
    }
    return names;
}

bool DockingManager::isPanelVisible(const std::string& name) const {
    auto it = m_Panels.find(name);
    return it != m_Panels.end() && it->second->isVisible();
}

void DockingManager::showPanel(const std::string& name, bool visible) {
    auto it = m_Panels.find(name);
    if (it != m_Panels.end()) {
        it->second->setVisible(visible);
    }
}

void DockingManager::renderPanels() {
    if (!m_DockspaceInitialized) {
        initializeDockspace();
    }

    // Begin dockspace
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Submit the DockSpace
    m_DockspaceID = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(m_DockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    // Render all panels
    for (const auto& [name, panel] : m_Panels) {
        if (panel) {
            panel->renderWindow(m_DockspaceID);
        }
    }

    ImGui::End();
}

void DockingManager::initializeDockspace() {
    if (m_DockspaceInitialized) return;

    // Set initial docking configuration
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    m_DockspaceID = ImGui::GetID("MainDockSpace");
    
    if (ImGui::DockBuilderGetNode(m_DockspaceID) == nullptr) {
        ImGui::DockBuilderRemoveNode(m_DockspaceID);
        ImGui::DockBuilderAddNode(m_DockspaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(m_DockspaceID, viewport->Size);
        
        setupDefaultLayout();
        
        ImGui::DockBuilderFinish(m_DockspaceID);
    }
    
    m_DockspaceInitialized = true;
    REBEL_LOG_INFO("Dockspace initialized");
}

void DockingManager::setupDefaultLayout() {
    // Split the dockspace into regions
    ImGuiID dockMainId = m_DockspaceID;
    ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, nullptr, &dockMainId);
    ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.2f, nullptr, &dockMainId);
    ImGuiID dockBottomId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.25f, nullptr, &dockMainId);

    // Dock windows into specific regions
    ImGui::DockBuilderDockWindow("Feature Tree", dockLeftId);
    ImGui::DockBuilderDockWindow("Properties", dockRightId);
    ImGui::DockBuilderDockWindow("Console", dockBottomId);
    ImGui::DockBuilderDockWindow("Layer Manager", dockRightId);
    ImGui::DockBuilderDockWindow("Toolbox", dockLeftId);
    
    REBEL_LOG_INFO("Default layout configured");
}

void DockingManager::resetToDefaultLayout() {
    m_DockspaceInitialized = false;
    initializeDockspace();
    
    // Notify layout change
    auto& eventBus = Core::EventBus::getInstance();
    eventBus.publish(LayoutChangedEvent{"reset_to_default"});
}

void DockingManager::saveLayout(const std::string& filename) {
    try {
        nlohmann::json layout;
        
        // Save panel visibility states
        nlohmann::json panels;
        for (const auto& [name, panel] : m_Panels) {
            panels[name] = {
                {"visible", panel->isVisible()},
                {"docked", panel->isDocked()}
            };
        }
        layout["panels"] = panels;
        
        // Save ImGui window states
        // TODO: Implement ImGui window state serialization
        
        // Write to file
        std::ofstream file(filename);
        file << layout.dump(4);
        
        REBEL_LOG_INFO("Layout saved to: {}", filename);
    }
    catch (const std::exception& e) {
        REBEL_LOG_ERROR("Failed to save layout: {}", e.what());
    }
}

void DockingManager::loadLayout(const std::string& filename) {
    try {
        // Read layout file
        std::ifstream file(filename);
        nlohmann::json layout;
        file >> layout;
        
        // Restore panel states
        if (layout.contains("panels")) {
            for (const auto& [name, state] : layout["panels"].items()) {
                auto it = m_Panels.find(name);
                if (it != m_Panels.end()) {
                    it->second->setVisible(state["visible"]);
                    it->second->setDocked(state["docked"]);
                }
            }
        }
        
        // TODO: Restore ImGui window states
        
        REBEL_LOG_INFO("Layout loaded from: {}", filename);
        
        // Notify layout change
        auto& eventBus = Core::EventBus::getInstance();
        eventBus.publish(LayoutChangedEvent{"load_layout"});
    }
    catch (const std::exception& e) {
        REBEL_LOG_ERROR("Failed to load layout: {}", e.what());
    }
}

} // namespace UI
} // namespace RebelCAD
