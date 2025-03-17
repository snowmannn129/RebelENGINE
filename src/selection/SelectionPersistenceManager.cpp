#include "graphics/SelectionPersistenceManager.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace RebelCAD {
namespace Graphics {

SelectionPersistenceManager::SelectionPersistenceManager(const SceneNode::Ptr& sceneRoot)
    : m_sceneRoot(sceneRoot)
{
}

bool SelectionPersistenceManager::saveState(
    const std::string& name,
    const std::vector<SceneNode::Ptr>& selection,
    const std::string& description
) {
    if (selection.empty()) {
        return false;
    }

    SavedState state{
        .name = name,
        .nodeIds = {},
        .description = description,
        .timestamp = getCurrentTimestamp(),
        .metadata = nlohmann::json::object()
    };

    // Generate unique IDs for each node
    state.nodeIds.reserve(selection.size());
    for (const auto& node : selection) {
        state.nodeIds.push_back(generateNodeId(node));
    }

    // Add metadata
    state.metadata["count"] = selection.size();
    state.metadata["version"] = "1.0.0";

    m_states[name] = std::move(state);
    return true;
}

std::vector<SceneNode::Ptr> SelectionPersistenceManager::loadState(
    const std::string& name
) {
    auto it = m_states.find(name);
    if (it == m_states.end()) {
        return {};
    }

    std::vector<SceneNode::Ptr> selection;
    selection.reserve(it->second.nodeIds.size());

    // Find nodes by their IDs
    for (const auto& id : it->second.nodeIds) {
        if (auto node = findNodeById(id)) {
            selection.push_back(node);
        }
    }

    return selection;
}

bool SelectionPersistenceManager::removeState(const std::string& name) {
    return m_states.erase(name) > 0;
}

std::vector<std::string> SelectionPersistenceManager::listStates() const {
    std::vector<std::string> names;
    names.reserve(m_states.size());
    for (const auto& [name, _] : m_states) {
        names.push_back(name);
    }
    return names;
}

std::string SelectionPersistenceManager::getStateDescription(
    const std::string& name
) const {
    auto it = m_states.find(name);
    return it != m_states.end() ? it->second.description : "";
}

bool SelectionPersistenceManager::saveStatesToDisk(const std::string& filepath) {
    try {
        nlohmann::json json;
        json["version"] = "1.0.0";
        json["timestamp"] = getCurrentTimestamp();
        
        // Serialize states
        auto& states = json["states"] = nlohmann::json::array();
        for (const auto& [name, state] : m_states) {
            states.push_back(serializeState(state));
        }

        // Write to file
        std::ofstream file(filepath);
        if (!file) {
            return false;
        }
        file << json.dump(2);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool SelectionPersistenceManager::loadStatesFromDisk(const std::string& filepath) {
    try {
        // Read file
        std::ifstream file(filepath);
        if (!file) {
            return false;
        }

        // Parse JSON
        nlohmann::json json;
        file >> json;

        // Clear existing states
        m_states.clear();

        // Load states
        for (const auto& stateJson : json["states"]) {
            auto state = deserializeState(stateJson);
            m_states[state.name] = std::move(state);
        }

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::string SelectionPersistenceManager::generateNodeId(
    const SceneNode::Ptr& node
) const {
    if (!node) return "";

    // Create unique ID from node path and transform hash
    std::string id = node->getFullPath();
    id += "#" + std::to_string(node->getTransformHash());

    // Add type info if available
    if (node->hasMeshData()) {
        id += ":mesh";
    }

    return id;
}

SceneNode::Ptr SelectionPersistenceManager::findNodeById(
    const std::string& id
) const {
    if (!m_sceneRoot || id.empty()) return nullptr;

    // Extract path from ID (everything before the '#')
    size_t hashPos = id.find('#');
    if (hashPos == std::string::npos) return nullptr;
    std::string path = id.substr(0, hashPos);

    // Find node by traversing scene graph
    SceneNode::Ptr result;
    traverseSceneGraph(m_sceneRoot, path, result);
    return result;
}

void SelectionPersistenceManager::traverseSceneGraph(
    const SceneNode::Ptr& node,
    const std::string& path,
    SceneNode::Ptr& result
) const {
    if (!node || result) return; // Stop if node is null or we found the result

    // Check if this node matches the path
    if (node->getFullPath() == path) {
        result = node;
        return;
    }

    // Recursively check children
    for (const auto& child : node->getChildren()) {
        traverseSceneGraph(child, path, result);
    }
}

nlohmann::json SelectionPersistenceManager::serializeState(
    const SavedState& state
) const {
    nlohmann::json json;
    json["name"] = state.name;
    json["nodeIds"] = state.nodeIds;
    json["description"] = state.description;
    json["timestamp"] = state.timestamp;
    json["metadata"] = state.metadata;
    return json;
}

SelectionPersistenceManager::SavedState SelectionPersistenceManager::deserializeState(
    const nlohmann::json& json
) const {
    SavedState state;
    state.name = json["name"].get<std::string>();
    state.nodeIds = json["nodeIds"].get<std::vector<std::string>>();
    state.description = json["description"].get<std::string>();
    state.timestamp = json["timestamp"].get<std::string>();
    state.metadata = json["metadata"];
    return state;
}

std::string SelectionPersistenceManager::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace Graphics
} // namespace RebelCAD
