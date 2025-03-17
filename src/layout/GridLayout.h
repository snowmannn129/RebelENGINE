#pragma once

#include <vector>
#include <memory>
#include "../Widget.h"
#include "../ImGuiWrapper.h"

namespace RebelCAD {
namespace UI {

/**
 * @brief Represents a grid-based layout manager for organizing widgets in rows and columns
 * 
 * The GridLayout class provides functionality to arrange widgets in a grid pattern,
 * with support for:
 * - Fixed or flexible column/row sizes
 * - Cell spanning across multiple rows/columns
 * - Cell alignment and padding
 * - Dynamic grid resizing
 */
class GridLayout {
public:
    /**
     * @brief Constructs a GridLayout with the specified number of rows and columns
     * @param rows Number of rows in the grid
     * @param cols Number of columns in the grid
     */
    GridLayout(int rows, int cols);
    ~GridLayout() = default;

    /**
     * @brief Adds a widget to the grid at the specified position
     * @param widget Widget to add
     * @param row Row index (0-based)
     * @param col Column index (0-based)
     * @param rowSpan Number of rows the widget spans (default: 1)
     * @param colSpan Number of columns the widget spans (default: 1)
     * @return true if widget was added successfully, false otherwise
     */
    bool addWidget(std::shared_ptr<Widget> widget, int row, int col, 
                  int rowSpan = 1, int colSpan = 1);

    /**
     * @brief Removes a widget from the grid
     * @param widget Widget to remove
     * @return true if widget was removed successfully, false otherwise
     */
    bool removeWidget(std::shared_ptr<Widget> widget);

    /**
     * @brief Sets the spacing between grid cells
     * @param spacing Spacing in pixels
     */
    void setSpacing(float spacing);

    /**
     * @brief Sets the padding inside grid cells
     * @param padding Padding in pixels
     */
    void setPadding(float padding);

    /**
     * @brief Updates the layout, recalculating positions and sizes
     * @param availableWidth Total available width
     * @param availableHeight Total available height
     */
    void update(float availableWidth, float availableHeight);

    /**
     * @brief Renders the grid and its contained widgets using ImGui
     */
    void render();

private:
    struct Cell {
        std::shared_ptr<Widget> widget;
        int rowSpan;
        int colSpan;
        bool isEmpty() const { return widget == nullptr; }
    };

    int m_rows;
    int m_cols;
    float m_spacing;
    float m_padding;
    std::vector<std::vector<Cell>> m_grid;
    std::vector<float> m_rowHeights;
    std::vector<float> m_colWidths;

    /**
     * @brief Calculates the size and position of each cell in the grid
     * @param availableWidth Total available width
     * @param availableHeight Total available height
     */
    void calculateLayout(float availableWidth, float availableHeight);

    /**
     * @brief Checks if a region in the grid is available for widget placement
     * @param row Starting row
     * @param col Starting column
     * @param rowSpan Number of rows to span
     * @param colSpan Number of columns to span
     * @return true if region is available, false otherwise
     */
    bool isRegionAvailable(int row, int col, int rowSpan, int colSpan) const;
};

} // namespace UI
} // namespace RebelCAD
