#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "graphics/Viewport.h"
#include "graphics/ViewportTypes.h"
#include "core/MemoryPool.h"

namespace RebelCAD {
namespace Graphics {


/**
 * @brief Represents a group of synchronized viewports
 */
class ViewSyncGroup {
public:
    /**
     * @brief Create a new sync group
     * @param name Group identifier
     */
    explicit ViewSyncGroup(const std::string& name);
    ~ViewSyncGroup();

    /**
     * @brief Add a viewport to the group
     * @param viewport Viewport to add
     * @return true if added successfully
     */
    bool addViewport(Viewport* viewport);

    /**
     * @brief Remove a viewport from the group
     * @param viewport Viewport to remove
     */
    void removeViewport(Viewport* viewport);

    /**
     * @brief Check if a viewport is in this group
     * @param viewport Viewport to check
     * @return true if viewport is in group
     */
    bool containsViewport(Viewport* viewport) const;

    /**
     * @brief Get the current view state
     * @return Current view state
     */
    ViewState getViewState() const;

    /**
     * @brief Update the view state and propagate to all viewports
     * @param state New view state
     */
    void updateViewState(const ViewState& state);

    /**
     * @brief Get the group name
     * @return Group identifier
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Get number of viewports in group
     * @return Viewport count
     */
    size_t getViewportCount() const { return m_viewports.size(); }

private:
    std::string m_name;
    std::vector<Viewport*> m_viewports;
    std::atomic<bool> m_updating;
    ViewState m_currentState;
};

/**
 * @brief Manages view synchronization across multiple viewport groups
 * 
 * The ViewSyncManager class provides a system for synchronizing view states
 * across multiple viewports. It supports:
 * - Group-based synchronization
 * - Efficient state propagation
 * - Thread-safe operations
 * - Hierarchical group management
 */
class ViewSyncManager {
public:
    /**
     * @brief Create a new sync manager
     */
    ViewSyncManager();
    ~ViewSyncManager();

    /**
     * @brief Create a new sync group
     * @param name Group identifier
     * @return Pointer to created group, nullptr if name exists
     */
    ViewSyncGroup* createGroup(const std::string& name);

    /**
     * @brief Remove a sync group
     * @param name Group identifier
     */
    void removeGroup(const std::string& name);

    /**
     * @brief Get a sync group by name
     * @param name Group identifier
     * @return Pointer to group, nullptr if not found
     */
    ViewSyncGroup* getGroup(const std::string& name);

    /**
     * @brief Add a viewport to a group
     * @param groupName Group identifier
     * @param viewport Viewport to add
     * @return true if added successfully
     */
    bool addToGroup(const std::string& groupName, Viewport* viewport);

    /**
     * @brief Remove a viewport from a group
     * @param groupName Group identifier
     * @param viewport Viewport to remove
     */
    void removeFromGroup(const std::string& groupName, Viewport* viewport);

    /**
     * @brief Update view state for a group
     * @param groupName Group identifier
     * @param state New view state
     */
    void updateGroupState(const std::string& groupName, const ViewState& state);

    /**
     * @brief Get groups containing a viewport
     * @param viewport Viewport to check
     * @return Vector of group names
     */
    std::vector<std::string> getViewportGroups(Viewport* viewport) const;

    /**
     * @brief Clear all sync groups
     */
    void clear();

private:
    // Memory management
    static Core::MemoryPool<ViewSyncGroup> s_groupPool;

    // Group management
    std::unordered_map<std::string, std::unique_ptr<ViewSyncGroup>> m_groups;
    mutable std::atomic<bool> m_updating;

    // Thread safety helpers
    bool tryLock() const;
    void unlock() const;
};

} // namespace Graphics
} // namespace RebelCAD
