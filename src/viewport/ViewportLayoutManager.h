#pragma once

#include "Viewport.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <nlohmann/json.hpp>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a viewport's position and size in a layout
 */
struct ViewportLayout {
    glm::vec2 position;     // Normalized position (0-1)
    glm::vec2 size;         // Normalized size (0-1)
    ViewportProjection defaultProjection;
    std::optional<ViewState> savedState;
    std::string name;       // Optional viewport name
};

/**
 * @brief Represents a complete layout template
 */
struct LayoutTemplate {
    std::string name;
    std::vector<ViewportLayout> viewports;
    glm::vec2 aspectRatio;  // Overall layout aspect ratio
    bool maintainAspectRatio;
};

/**
 * @brief Manages viewport layout templates and state persistence
 */
class ViewportLayoutManager {
public:
    /**
     * @brief Constructor
     */
    ViewportLayoutManager();

    /**
     * @brief Creates a new layout template
     * @param name Template name
     * @param viewports Vector of viewport layouts
     * @param aspectRatio Overall layout aspect ratio
     * @param maintainAspectRatio Whether to maintain aspect ratio during resizing
     * @return true if template was created successfully
     */
    bool createTemplate(
        const std::string& name,
        const std::vector<ViewportLayout>& viewports,
        const glm::vec2& aspectRatio = glm::vec2(16, 9),
        bool maintainAspectRatio = true
    );

    /**
     * @brief Loads a layout template
     * @param name Template name
     * @return Optional template if found
     */
    std::optional<LayoutTemplate> loadTemplate(const std::string& name) const;

    /**
     * @brief Saves current viewport states to a named layout
     * @param name Layout name
     * @param viewports Vector of viewports to save
     * @return true if states were saved successfully
     */
    bool saveViewportStates(
        const std::string& name,
        const std::vector<std::shared_ptr<Viewport>>& viewports
    );

    /**
     * @brief Loads and applies saved viewport states
     * @param name Layout name
     * @param viewports Vector of viewports to update
     * @return true if states were loaded and applied successfully
     */
    bool loadViewportStates(
        const std::string& name,
        std::vector<std::shared_ptr<Viewport>>& viewports
    );

    /**
     * @brief Creates a standard layout template
     * @param type Standard layout type
     * @param name Optional custom name for the template
     * @return true if template was created successfully
     */
    bool createStandardTemplate(
        StandardLayoutType type,
        const std::string& name = ""
    );

    /**
     * @brief Applies a template to a set of viewports
     * @param templateName Name of template to apply
     * @param viewports Vector of viewports to update
     * @param containerSize Size of container in pixels
     * @return true if template was applied successfully
     */
    bool applyTemplate(
        const std::string& templateName,
        std::vector<std::shared_ptr<Viewport>>& viewports,
        const glm::vec2& containerSize
    );

    /**
     * @brief Lists all available layout templates
     * @return Vector of template names
     */
    std::vector<std::string> listTemplates() const;

    /**
     * @brief Removes a layout template
     * @param name Template name
     * @return true if template was removed
     */
    bool removeTemplate(const std::string& name);

    /**
     * @brief Saves all templates to disk
     * @param filepath Path to save file
     * @return true if save was successful
     */
    bool saveTemplates(const std::string& filepath) const;

    /**
     * @brief Loads templates from disk
     * @param filepath Path to load file
     * @return true if load was successful
     */
    bool loadTemplates(const std::string& filepath);

private:
    // Standard layout generators
    std::vector<ViewportLayout> generateSingleLayout();
    std::vector<ViewportLayout> generateDoubleLayout();
    std::vector<ViewportLayout> generateTripleLayout();
    std::vector<ViewportLayout> generateQuadLayout();
    std::vector<ViewportLayout> generate3x2Layout();
    std::vector<ViewportLayout> generate2x3Layout();

    // Helper methods
    glm::vec2 calculatePixelPosition(
        const glm::vec2& normalizedPos,
        const glm::vec2& containerSize
    ) const;
    
    glm::vec2 calculatePixelSize(
        const glm::vec2& normalizedSize,
        const glm::vec2& containerSize
    ) const;

    // Template storage
    std::unordered_map<std::string, LayoutTemplate> m_templates;

    // JSON serialization helpers
    nlohmann::json serializeTemplate(const LayoutTemplate& templ) const;
    LayoutTemplate deserializeTemplate(const nlohmann::json& json) const;
    nlohmann::json serializeViewState(const ViewState& state) const;
    ViewState deserializeViewState(const nlohmann::json& json) const;

    // Helper methods
    std::string getDefaultTemplateName(StandardLayoutType type) const;
};

} // namespace Graphics
} // namespace RebelCAD
