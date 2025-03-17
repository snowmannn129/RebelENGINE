#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <array>
#include <glm/glm.hpp>
#include "graphics/Viewport.h"
#include "graphics/ViewportTypes.h"
#include "core/MemoryPool.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a cell in the viewport grid
 */
struct ViewportCell {
    int row;
    int col;
    int rowSpan;
    int colSpan;
    
    ViewportCell(int r = 0, int c = 0, int rs = 1, int cs = 1)
        : row(r), col(c), rowSpan(rs), colSpan(cs) {}
};

/**
 * @brief Represents a preset layout configuration
 */
struct ViewportLayoutPreset {
    std::string name;                            ///< Preset identifier
    int rows;                                    ///< Number of grid rows
    int cols;                                    ///< Number of grid columns
    std::vector<ViewportCell> cells;             ///< Cell configurations
    std::vector<ViewportProjection> projections; ///< Projection type per viewport
    std::vector<ViewportView> views;             ///< View orientation per viewport
    
    ViewportLayoutPreset() = default;
    ViewportLayoutPreset(const std::string& n, int r, int c)
        : name(n), rows(r), cols(c) {}
};

/**
 * @brief Manages a grid-based multi-viewport layout system
 * 
 * The ViewportLayout class provides a flexible grid-based system for managing
 * multiple viewports. It supports dynamic layout modifications, preset configurations,
 * and efficient viewport state management.
 */
class ViewportLayout {
public:
    /**
     * @brief Construct a new ViewportLayout
     * 
     * @param width Initial window width in pixels
     * @param height Initial window height in pixels
     */
    ViewportLayout(int width, int height);
    
    /**
     * @brief Destroy the ViewportLayout
     */
    ~ViewportLayout();

    /**
     * @brief Set the window dimensions
     * 
     * @param width New window width in pixels
     * @param height New window height in pixels
     */
    void setDimensions(int width, int height);

    /**
     * @brief Set the grid dimensions
     * 
     * @param rows Number of rows in the grid
     * @param cols Number of columns in the grid
     */
    void setGridSize(int rows, int cols);

    /**
     * @brief Add a viewport to the layout
     * 
     * @param cell Grid cell configuration for the viewport
     * @param projection Initial projection type
     * @param view Initial standard view
     * @return Pointer to the created viewport
     */
    Viewport* addViewport(const ViewportCell& cell, 
                         ViewportProjection projection = ViewportProjection::Perspective,
                         ViewportView view = ViewportView::Isometric);

    /**
     * @brief Remove a viewport from the layout
     * 
     * @param viewport Pointer to the viewport to remove
     */
    void removeViewport(Viewport* viewport);

    /**
     * @brief Get the viewport at a specific grid position
     * 
     * @param row Grid row
     * @param col Grid column
     * @return Pointer to the viewport if found, nullptr otherwise
     */
    Viewport* getViewport(int row, int col) const;

    /**
     * @brief Apply a preset layout configuration
     * 
     * @param preset Layout preset to apply
     */
    void applyPreset(const ViewportLayoutPreset& preset);

    /**
     * @brief Save current layout as a preset
     * 
     * @param name Name for the preset
     * @return ViewportLayoutPreset object representing current layout
     */
    ViewportLayoutPreset saveAsPreset(const std::string& name) const;

    /**
     * @brief Get the screen coordinates for a grid cell
     * 
     * @param cell Grid cell configuration
     * @return Rectangle coordinates (x, y, width, height)
     */
    glm::vec4 getCellRect(const ViewportCell& cell) const;

    /**
     * @brief Update all viewports
     * This triggers a redraw of all viewports in the layout
     */
    void updateAll();

    /**
     * @brief Synchronize a group of viewports
     * 
     * @param viewports Vector of viewport pointers to synchronize
     * @param sync true to enable synchronization, false to disable
     */
    void synchronizeViewports(const std::vector<Viewport*>& viewports, bool sync = true);

private:
    /**
     * @brief Recalculate viewport dimensions based on grid layout
     */
    void recalculateViewportDimensions();

    /**
     * @brief Check if a cell configuration is valid
     * 
     * @param cell Cell configuration to check
     * @return true if valid, false otherwise
     */
    bool isValidCell(const ViewportCell& cell) const;

    /**
     * @brief Check if a cell overlaps with existing viewports
     * 
     * @param cell Cell configuration to check
     * @return true if overlapping, false otherwise
     */
    bool hasOverlap(const ViewportCell& cell) const;

    // Window dimensions
    std::atomic<int> m_width;
    std::atomic<int> m_height;

    // Grid configuration
    int m_rows;
    int m_cols;

    // Viewport management
    struct ViewportEntry {
        std::unique_ptr<Viewport> viewport;
        ViewportCell cell;
    };
    std::vector<ViewportEntry> m_viewports;

    // Thread safety
    mutable std::atomic<bool> m_updating;

    // Memory management
    static Core::MemoryPool<Viewport> s_viewportPool;
};

} // namespace Graphics
} // namespace RebelCAD
