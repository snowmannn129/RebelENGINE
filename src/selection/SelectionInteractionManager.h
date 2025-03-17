#pragma once

#pragma once

#include "graphics/SelectionHistoryManager.h"
#include "graphics/SelectionFilterManager.h"
#include "graphics/SelectionGroupManager.h"
#include "graphics/SelectionPersistenceManager.h"
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Graphics {

// Forward declarations
class ObjectPicker;
class Viewport;

/**
 * @brief Selection mode types
 */
enum class SelectionMode {
    Point,      ///< Single point selection
    Box,        ///< Rectangular region selection
    Lasso,      ///< Freeform region selection
    Paint,      ///< Brush-based selection
    Smart       ///< Feature-based smart selection
};

/**
 * @brief Selection modifier flags
 */
enum class SelectionModifier {
    None,       ///< Replace current selection
    Add,        ///< Add to current selection
    Subtract,   ///< Remove from current selection
    Intersect   ///< Keep only objects in both selections
};

/**
 * @brief Configuration for selection behavior
 */
struct SelectionConfig {
    float pointRadius = 5.0f;           ///< Radius for point selection in pixels
    float paintBrushSize = 20.0f;       ///< Size of paint brush in pixels
    float smartSelectionTolerance = 0.1f;///< Tolerance for smart selection
    bool autoCompleteSmartSelection = true; ///< Auto-complete smart selection
    bool visualFeedback = true;         ///< Show visual feedback during selection
    bool previewSelection = true;       ///< Preview selection before committing
};

/**
 * @brief Manages selection interaction and modes
 */
class SelectionInteractionManager {
public:
    /**
     * @brief Constructor
     * @param picker Object picker for selection operations
     * @param viewport Viewport for coordinate conversion
     */
    SelectionInteractionManager(
        std::shared_ptr<ObjectPicker> picker,
        std::shared_ptr<Viewport> viewport
    );

    /**
     * @brief Sets the selection mode
     * @param mode New selection mode
     */
    void setMode(SelectionMode mode);

    /**
     * @brief Gets the current selection mode
     * @return Current mode
     */
    SelectionMode getMode() const { return m_currentMode; }

    /**
     * @brief Sets the selection modifier
     * @param modifier New modifier
     */
    void setModifier(SelectionModifier modifier);

    /**
     * @brief Gets the current selection modifier
     * @return Current modifier
     */
    SelectionModifier getModifier() const { return m_currentModifier; }

    /**
     * @brief Starts a selection operation
     * @param x Starting X coordinate
     * @param y Starting Y coordinate
     */
    void startSelection(float x, float y);

    /**
     * @brief Updates the current selection
     * @param x Current X coordinate
     * @param y Current Y coordinate
     */
    void updateSelection(float x, float y);

    /**
     * @brief Completes the current selection
     * @param x Final X coordinate
     * @param y Final Y coordinate
     * @return Vector of selected objects
     */
    std::vector<SceneNode::Ptr> completeSelection(float x, float y);

    /**
     * @brief Cancels the current selection operation
     */
    void cancelSelection();

    /**
     * @brief Sets the selection configuration
     * @param config New configuration
     */
    void setConfig(const SelectionConfig& config);

    /**
     * @brief Gets the current selection configuration
     * @return Current configuration
     */
    const SelectionConfig& getConfig() const { return m_config; }

    /**
     * @brief Sets callback for selection changes
     * @param callback Function to call when selection changes
     */
    void setSelectionChangeCallback(
        std::function<void(const std::vector<SceneNode::Ptr>&)> callback
    );

    /**
     * @brief Sets callback for selection preview
     * @param callback Function to call with preview selection
     */
    void setPreviewCallback(
        std::function<void(const std::vector<SceneNode::Ptr>&)> callback
    );

    /**
     * @brief Sets callback for visual feedback
     * @param callback Function to call with feedback geometry
     */
    void setFeedbackCallback(
        std::function<void(const std::vector<glm::vec2>&)> callback
    );

private:
    // Selection operation implementations
    std::vector<SceneNode::Ptr> handlePointSelection(float x, float y);
    std::vector<SceneNode::Ptr> handleBoxSelection(
        float startX, float startY,
        float endX, float endY
    );
    std::vector<SceneNode::Ptr> handleLassoSelection(
        const std::vector<glm::vec2>& points
    );
    std::vector<SceneNode::Ptr> handlePaintSelection(
        const std::vector<glm::vec2>& brushPoints
    );
    std::vector<SceneNode::Ptr> handleSmartSelection(float x, float y);

    // Helper methods
    void updatePreview();
    void updateFeedback();
    std::vector<SceneNode::Ptr> applyModifier(
        const std::vector<SceneNode::Ptr>& newSelection
    );
    bool isValidSelection() const;
    void clearSelectionState();

    // Core components
    std::shared_ptr<ObjectPicker> m_picker;
    std::shared_ptr<Viewport> m_viewport;
    SelectionMode m_currentMode;
    SelectionModifier m_currentModifier;
    SelectionConfig m_config;

    // Selection state
    struct {
        bool active = false;
        float startX = 0.0f;
        float startY = 0.0f;
        float currentX = 0.0f;
        float currentY = 0.0f;
        std::vector<glm::vec2> points;
        std::vector<SceneNode::Ptr> preview;
    } m_selectionState;

    // Smart selection state
    struct {
        std::vector<SceneNode::Ptr> seedObjects;
        float similarityThreshold;
        bool autoComplete;
    } m_smartSelectionState;

    // Callbacks
    std::function<void(const std::vector<SceneNode::Ptr>&)> m_selectionChangeCallback;
    std::function<void(const std::vector<SceneNode::Ptr>&)> m_previewCallback;
    std::function<void(const std::vector<glm::vec2>&)> m_feedbackCallback;

    // Selection management
    std::vector<SceneNode::Ptr> m_currentSelection;
    std::unique_ptr<SelectionHistoryManager> m_history;
    std::unique_ptr<SelectionFilterManager> m_filterManager;
    std::unique_ptr<SelectionGroupManager> m_groupManager;
    std::unique_ptr<SelectionPersistenceManager> m_persistenceManager;

    // Helper methods
    void recordSelectionOperation(
        const std::vector<SceneNode::Ptr>& newSelection,
        const std::string& description
    );
    
    std::vector<SceneNode::Ptr> applyFilters(
        const std::vector<SceneNode::Ptr>& selection
    ) const;

    // Group operations
    bool createSelectionGroup(const std::string& name);
    bool addSelectionToGroup(const std::string& name);
    bool removeSelectionFromGroup(const std::string& name);
    bool selectGroup(const std::string& name);

    // Persistence operations
    bool saveSelectionState(const std::string& name, const std::string& description = "");
    bool loadSelectionState(const std::string& name);
    bool saveSelectionsToDisk(const std::string& filepath);
    bool loadSelectionsFromDisk(const std::string& filepath);
};

} // namespace Graphics
} // namespace RebelCAD
