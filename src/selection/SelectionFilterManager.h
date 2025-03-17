#pragma once

#include "graphics/SceneNode.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a selection filter
 */
struct SelectionFilter {
    std::string name;                                    ///< Filter name
    std::function<bool(const SceneNode::Ptr&)> predicate;///< Filter function
    bool enabled = true;                                 ///< Filter state
    std::string description;                            ///< Filter description
};

/**
 * @brief Manages selection filters and filtering operations
 */
class SelectionFilterManager {
public:
    /**
     * @brief Constructor
     */
    SelectionFilterManager();

    /**
     * @brief Adds a new filter
     * @param name Filter name
     * @param predicate Filter function
     * @param description Filter description
     * @return true if filter was added successfully
     */
    bool addFilter(
        const std::string& name,
        std::function<bool(const SceneNode::Ptr&)> predicate,
        const std::string& description = ""
    );

    /**
     * @brief Removes a filter
     * @param name Filter name
     * @return true if filter was removed
     */
    bool removeFilter(const std::string& name);

    /**
     * @brief Enables or disables a filter
     * @param name Filter name
     * @param enabled New state
     * @return true if filter state was changed
     */
    bool setFilterEnabled(const std::string& name, bool enabled);

    /**
     * @brief Gets a filter's enabled state
     * @param name Filter name
     * @return true if filter is enabled
     */
    bool isFilterEnabled(const std::string& name) const;

    /**
     * @brief Gets a filter's description
     * @param name Filter name
     * @return Filter description
     */
    std::string getFilterDescription(const std::string& name) const;

    /**
     * @brief Lists all available filters
     * @return Vector of filter names
     */
    std::vector<std::string> listFilters() const;

    /**
     * @brief Applies all enabled filters to a selection
     * @param selection Selection to filter
     * @return Filtered selection
     */
    std::vector<SceneNode::Ptr> applyFilters(
        const std::vector<SceneNode::Ptr>& selection
    ) const;

    /**
     * @brief Applies a specific filter to a selection
     * @param name Filter name
     * @param selection Selection to filter
     * @return Filtered selection
     */
    std::vector<SceneNode::Ptr> applyFilter(
        const std::string& name,
        const std::vector<SceneNode::Ptr>& selection
    ) const;

    /**
     * @brief Creates standard filters for common operations
     */
    void createStandardFilters();

private:
    std::unordered_map<std::string, SelectionFilter> m_filters;

    // Standard filter implementations
    static bool filterVisibleObjects(const SceneNode::Ptr& node);
    static bool filterSelectableObjects(const SceneNode::Ptr& node);
    static bool filterMeshObjects(const SceneNode::Ptr& node);
    static bool filterGroupNodes(const SceneNode::Ptr& node);
    static bool filterLeafNodes(const SceneNode::Ptr& node);
};

} // namespace Graphics
} // namespace RebelCAD
