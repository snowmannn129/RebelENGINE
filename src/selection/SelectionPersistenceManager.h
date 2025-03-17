#pragma once

#include "graphics/SceneNode.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Manages persistence of selection states
 */
class SelectionPersistenceManager {
public:
    /**
     * @brief Constructor
     * @param sceneRoot Root node of the scene graph
     */
    explicit SelectionPersistenceManager(const SceneNode::Ptr& sceneRoot);

    /**
     * @brief Saves current selection state
     * @param name State name
     * @param selection Current selection
     * @param description Optional description
     * @return true if state was saved
     */
    bool saveState(
        const std::string& name,
        const std::vector<SceneNode::Ptr>& selection,
        const std::string& description = ""
    );

    /**
     * @brief Loads a saved selection state
     * @param name State name
     * @return Saved selection or empty vector if not found
     */
    std::vector<SceneNode::Ptr> loadState(const std::string& name);

    /**
     * @brief Removes a saved state
     * @param name State name
     * @return true if state was removed
     */
    bool removeState(const std::string& name);

    /**
     * @brief Lists all saved states
     * @return Vector of state names
     */
    std::vector<std::string> listStates() const;

    /**
     * @brief Gets a state's description
     * @param name State name
     * @return State description
     */
    std::string getStateDescription(const std::string& name) const;

    /**
     * @brief Saves all states to disk
     * @param filepath Path to save file
     * @return true if states were saved
     */
    bool saveStatesToDisk(const std::string& filepath);

    /**
     * @brief Loads states from disk
     * @param filepath Path to save file
     * @return true if states were loaded
     */
    bool loadStatesFromDisk(const std::string& filepath);

private:
    /**
     * @brief Represents a saved selection state
     */
    struct SavedState {
        std::string name;                            ///< State name
        std::vector<std::string> nodeIds;           ///< Node identifiers
        std::string description;                    ///< State description
        std::string timestamp;                      ///< Save timestamp
        nlohmann::json metadata;                    ///< Additional metadata
    };

    std::unordered_map<std::string, SavedState> m_states;
    SceneNode::Ptr m_sceneRoot;                    ///< Root of scene graph for traversal

    // Helper methods
    std::string generateNodeId(const SceneNode::Ptr& node) const;
    SceneNode::Ptr findNodeById(const std::string& id) const;
    nlohmann::json serializeState(const SavedState& state) const;
    SavedState deserializeState(const nlohmann::json& json) const;
    std::string getCurrentTimestamp() const;
    void traverseSceneGraph(
        const SceneNode::Ptr& node,
        const std::string& id,
        SceneNode::Ptr& result
    ) const;
};

} // namespace Graphics
} // namespace RebelCAD
