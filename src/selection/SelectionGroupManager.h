#pragma once

#include "graphics/SceneNode.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a selection group
 */
struct SelectionGroup {
    std::string name;                            ///< Group name
    std::vector<SceneNode::Ptr> nodes;          ///< Group members
    bool visible = true;                        ///< Group visibility
    bool locked = false;                        ///< Group lock state
    std::string description;                    ///< Group description
    std::unordered_map<std::string, std::string> metadata; ///< Additional metadata
};

/**
 * @brief Manages selection groups and group operations
 */
class SelectionGroupManager {
public:
    /**
     * @brief Constructor
     */
    SelectionGroupManager();

    /**
     * @brief Creates a new group
     * @param name Group name
     * @param nodes Initial group members
     * @param description Group description
     * @return true if group was created successfully
     */
    bool createGroup(
        const std::string& name,
        const std::vector<SceneNode::Ptr>& nodes,
        const std::string& description = ""
    );

    /**
     * @brief Removes a group
     * @param name Group name
     * @return true if group was removed
     */
    bool removeGroup(const std::string& name);

    /**
     * @brief Adds nodes to a group
     * @param name Group name
     * @param nodes Nodes to add
     * @return true if nodes were added
     */
    bool addToGroup(
        const std::string& name,
        const std::vector<SceneNode::Ptr>& nodes
    );

    /**
     * @brief Removes nodes from a group
     * @param name Group name
     * @param nodes Nodes to remove
     * @return true if nodes were removed
     */
    bool removeFromGroup(
        const std::string& name,
        const std::vector<SceneNode::Ptr>& nodes
    );

    /**
     * @brief Gets a group's members
     * @param name Group name
     * @return Vector of group members
     */
    std::vector<SceneNode::Ptr> getGroupMembers(const std::string& name) const;

    /**
     * @brief Gets groups containing a node
     * @param node Node to check
     * @return Vector of group names
     */
    std::vector<std::string> getNodeGroups(const SceneNode::Ptr& node) const;

    /**
     * @brief Sets a group's visibility
     * @param name Group name
     * @param visible New visibility state
     * @return true if state was changed
     */
    bool setGroupVisible(const std::string& name, bool visible);

    /**
     * @brief Sets a group's lock state
     * @param name Group name
     * @param locked New lock state
     * @return true if state was changed
     */
    bool setGroupLocked(const std::string& name, bool locked);

    /**
     * @brief Lists all available groups
     * @return Vector of group names
     */
    std::vector<std::string> listGroups() const;

    /**
     * @brief Gets a group's description
     * @param name Group name
     * @return Group description
     */
    std::string getGroupDescription(const std::string& name) const;

    /**
     * @brief Sets a group's metadata
     * @param name Group name
     * @param key Metadata key
     * @param value Metadata value
     * @return true if metadata was set
     */
    bool setGroupMetadata(
        const std::string& name,
        const std::string& key,
        const std::string& value
    );

    /**
     * @brief Gets a group's metadata
     * @param name Group name
     * @param key Metadata key
     * @return Metadata value or empty string if not found
     */
    std::string getGroupMetadata(
        const std::string& name,
        const std::string& key
    ) const;

    /**
     * @brief Sets callback for group changes
     * @param callback Function to call when groups change
     */
    void setGroupChangeCallback(
        std::function<void(const std::string&)> callback
    );

private:
    std::unordered_map<std::string, SelectionGroup> m_groups;
    std::function<void(const std::string&)> m_groupChangeCallback;

    // Helper methods
    void notifyGroupChange(const std::string& name);
    bool isValidGroup(const std::string& name) const;
};

} // namespace Graphics
} // namespace RebelCAD
