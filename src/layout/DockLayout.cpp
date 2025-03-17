#include "ui/layouts/DockLayout.h"
#include "ui/ImGuiWrapper.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace RebelCAD {
namespace UI {

DockLayout::DockLayout() : m_MainDockID(0), m_Initialized(false) {
    // Initialize default split ratios
    m_SplitRatios[DockZone::Left] = 0.2f;
    m_SplitRatios[DockZone::Right] = 0.2f;
    m_SplitRatios[DockZone::Top] = 0.2f;
    m_SplitRatios[DockZone::Bottom] = 0.2f;
}

DockLayout::~DockLayout() {
    // Clean up dock nodes
    for (const auto& node : m_DockNodes) {
        if (DockBuilderGetNode(node.id)) {
            DockBuilderRemoveNode(node.id);
        }
    }
}

ImGuiID DockLayout::dockWidget(std::shared_ptr<Widget> widget, DockZone zone, float size) {
    if (!widget) return 0;

    // Initialize dockspace if needed
    if (!m_Initialized) {
        initialize();
    }

    // Remove widget if it's already docked
    undockWidget(widget);

    // Create new dock node
    ImGuiID nodeId = createDockNode(zone, size);
    if (nodeId == 0) return 0;

    // Store node information
    DockNode node{nodeId, zone, size, widget};
    m_DockNodes.push_back(node);

    return nodeId;
}

void DockLayout::undockWidget(std::shared_ptr<Widget> widget) {
    if (!widget) return;

    auto it = std::find_if(m_DockNodes.begin(), m_DockNodes.end(),
        [&widget](const DockNode& node) { return node.widget == widget; });

    if (it != m_DockNodes.end()) {
        if (DockBuilderGetNode(it->id)) {
            DockBuilderRemoveNode(it->id);
        }
        m_DockNodes.erase(it);
    }
}

bool DockLayout::isWidgetDocked(std::shared_ptr<Widget> widget) const {
    if (!widget) return false;

    return std::find_if(m_DockNodes.begin(), m_DockNodes.end(),
        [&widget](const DockNode& node) { return node.widget == widget; }) != m_DockNodes.end();
}

DockZone DockLayout::getWidgetZone(std::shared_ptr<Widget> widget) const {
    if (!widget) return DockZone::Center;

    auto it = std::find_if(m_DockNodes.begin(), m_DockNodes.end(),
        [&widget](const DockNode& node) { return node.widget == widget; });

    return it != m_DockNodes.end() ? it->zone : DockZone::Center;
}

void DockLayout::setSplitRatio(DockZone zone, float ratio) {
    // Clamp ratio between 0 and 1
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    m_SplitRatios[zone] = ratio;
    updateSplitRatios();
}

float DockLayout::getSplitRatio(DockZone zone) const {
    auto it = m_SplitRatios.find(zone);
    return it != m_SplitRatios.end() ? it->second : 0.2f; // Default ratio
}

void DockLayout::render() {
    if (!m_Initialized) {
        initialize();
    }

    // Create dockspace
    ImGuiID dockspaceId = DockSpace("MainDockSpace");

    // Render each docked widget
    for (const auto& node : m_DockNodes) {
        if (node.widget) {
            ImGui::SetNextWindowDockID(node.id, ImGuiCond_FirstUseEver);
            node.widget->render();
        }
    }
}

bool DockLayout::saveConfiguration(const std::string& filename) {
    try {
        nlohmann::json config;

        // Save split ratios
        nlohmann::json ratios;
        for (const auto& [zone, ratio] : m_SplitRatios) {
            ratios[std::to_string(static_cast<int>(zone))] = ratio;
        }
        config["splitRatios"] = ratios;

        // Save dock nodes
        nlohmann::json nodes;
        for (const auto& node : m_DockNodes) {
            nlohmann::json nodeData;
            nodeData["id"] = node.id;
            nodeData["zone"] = static_cast<int>(node.zone);
            nodeData["size"] = node.size;
            nodes.push_back(nodeData);
        }
        config["dockNodes"] = nodes;

        // Write to file
        std::ofstream file(filename);
        file << config.dump(4);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool DockLayout::loadConfiguration(const std::string& filename) {
    try {
        std::ifstream file(filename);
        nlohmann::json config;
        file >> config;

        // Load split ratios
        auto ratios = config["splitRatios"];
        for (auto it = ratios.begin(); it != ratios.end(); ++it) {
            DockZone zone = static_cast<DockZone>(std::stoi(it.key()));
            setSplitRatio(zone, it.value());
        }

        // Reset current layout
        resetToDefault();

        // Recreate dock nodes
        for (const auto& nodeData : config["dockNodes"]) {
            DockZone zone = static_cast<DockZone>(nodeData["zone"]);
            float size = nodeData["size"];
            createDockNode(zone, size);
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

void DockLayout::resetToDefault() {
    // Remove all dock nodes
    for (const auto& node : m_DockNodes) {
        if (DockBuilderGetNode(node.id)) {
            DockBuilderRemoveNode(node.id);
        }
    }
    m_DockNodes.clear();

    // Reset split ratios to defaults
    m_SplitRatios[DockZone::Left] = 0.2f;
    m_SplitRatios[DockZone::Right] = 0.2f;
    m_SplitRatios[DockZone::Top] = 0.2f;
    m_SplitRatios[DockZone::Bottom] = 0.2f;

    // Reinitialize dockspace
    m_Initialized = false;
    initialize();
}

void DockLayout::initialize() {
    if (m_Initialized) return;

    // Create main dockspace
    m_MainDockID = DockBuilderAddNode(0, ImGuiDockNodeFlags_DockSpace);
    DockBuilderSetNodeSize(m_MainDockID, ImGui::GetMainViewport()->Size);

    setupDockspace();
    m_Initialized = true;
}

void DockLayout::setupDockspace() {
    // Create dock splits for each zone
    ImGuiID remainingId = m_MainDockID;
    ImGuiID dockLeft = 0, dockRight = 0, dockUp = 0, dockDown = 0;

    // Split left
    if (m_SplitRatios[DockZone::Left] > 0.0f) {
        remainingId = DockBuilderSplitNode(remainingId, ImGuiDir_Left, m_SplitRatios[DockZone::Left], &dockLeft, &remainingId);
    }

    // Split right
    if (m_SplitRatios[DockZone::Right] > 0.0f) {
        remainingId = DockBuilderSplitNode(remainingId, ImGuiDir_Right, m_SplitRatios[DockZone::Right], &dockRight, &remainingId);
    }

    // Split top
    if (m_SplitRatios[DockZone::Top] > 0.0f) {
        remainingId = DockBuilderSplitNode(remainingId, ImGuiDir_Up, m_SplitRatios[DockZone::Top], &dockUp, &remainingId);
    }

    // Split bottom
    if (m_SplitRatios[DockZone::Bottom] > 0.0f) {
        remainingId = DockBuilderSplitNode(remainingId, ImGuiDir_Down, m_SplitRatios[DockZone::Bottom], &dockDown, &remainingId);
    }

    DockBuilderFinish(m_MainDockID);
}

ImGuiID DockLayout::createDockNode(DockZone zone, float size) {
    // Set split ratio based on size and viewport dimensions
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float ratio;

    switch (zone) {
        case DockZone::Left:
        case DockZone::Right:
            ratio = size / viewport->Size.x;
            break;
        case DockZone::Top:
        case DockZone::Bottom:
            ratio = size / viewport->Size.y;
            break;
        default:
            return 0;
    }

    setSplitRatio(zone, ratio);
    
    // Create new dock node
    ImGuiID nodeId = DockBuilderAddNode(0, ImGuiDockNodeFlags_None);
    if (nodeId == 0) return 0;

    // Set node size
    DockBuilderSetNodeSize(nodeId, ImVec2(size, size));

    return nodeId;
}

void DockLayout::updateSplitRatios() {
    if (!m_Initialized) return;

    // Recreate dockspace with new ratios
    DockBuilderRemoveNode(m_MainDockID);
    m_MainDockID = DockBuilderAddNode(0, ImGuiDockNodeFlags_DockSpace);
    DockBuilderSetNodeSize(m_MainDockID, ImGui::GetMainViewport()->Size);
    setupDockspace();
}

} // namespace UI
} // namespace RebelCAD
