#include "../../include/ui/layouts/GridLayout.h"
#include <algorithm>
#include <stdexcept>

namespace RebelCAD {
namespace UI {

GridLayout::GridLayout(int rows, int cols) 
    : m_rows(rows)
    , m_cols(cols)
    , m_spacing(5.0f)
    , m_padding(2.0f) {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
    
    // Initialize grid with empty cells
    m_grid.resize(rows, std::vector<Cell>(cols));
    m_rowHeights.resize(rows, 0.0f);
    m_colWidths.resize(cols, 0.0f);
}

bool GridLayout::addWidget(std::shared_ptr<Widget> widget, int row, int col,
                         int rowSpan, int colSpan) {
    // Validate parameters
    if (!widget || row < 0 || col < 0 || row >= m_rows || col >= m_cols ||
        rowSpan <= 0 || colSpan <= 0 || 
        row + rowSpan > m_rows || col + colSpan > m_cols) {
        return false;
    }

    // Check if target region is available
    if (!isRegionAvailable(row, col, rowSpan, colSpan)) {
        return false;
    }

    // Place widget in all spanned cells
    for (int r = row; r < row + rowSpan; ++r) {
        for (int c = col; c < col + colSpan; ++c) {
            m_grid[r][c].widget = widget;
            m_grid[r][c].rowSpan = rowSpan;
            m_grid[r][c].colSpan = colSpan;
        }
    }

    return true;
}

bool GridLayout::removeWidget(std::shared_ptr<Widget> widget) {
    bool found = false;
    
    // Find and remove the widget from all cells it occupies
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_grid[r][c].widget == widget) {
                found = true;
                // Clear all cells occupied by this widget
                int rowSpan = m_grid[r][c].rowSpan;
                int colSpan = m_grid[r][c].colSpan;
                for (int sr = r; sr < r + rowSpan; ++sr) {
                    for (int sc = c; sc < c + colSpan; ++sc) {
                        m_grid[sr][sc] = Cell();
                    }
                }
                // Skip the rest of this widget's cells
                c += colSpan - 1;
            }
        }
    }

    return found;
}

void GridLayout::setSpacing(float spacing) {
    m_spacing = std::max(0.0f, spacing);
}

void GridLayout::setPadding(float padding) {
    m_padding = std::max(0.0f, padding);
}

void GridLayout::update(float availableWidth, float availableHeight) {
    calculateLayout(availableWidth, availableHeight);
}

void GridLayout::render() {
    ImGui::BeginGroup();
    
    float currentY = 0;
    for (int r = 0; r < m_rows; ++r) {
        float currentX = 0;
        float rowHeight = m_rowHeights[r];
        
        for (int c = 0; c < m_cols; ++c) {
            const auto& cell = m_grid[r][c];
            float colWidth = m_colWidths[c];
            
            // Only render if this is the top-left cell of a widget
            if (cell.widget && 
                (r == 0 || m_grid[r-1][c].widget != cell.widget) &&
                (c == 0 || m_grid[r][c-1].widget != cell.widget)) {
                
                // Calculate total width and height for spanning
                float totalWidth = colWidth;
                float totalHeight = rowHeight;
                for (int sc = 1; sc < cell.colSpan; ++sc) {
                    totalWidth += m_spacing + m_colWidths[c + sc];
                }
                for (int sr = 1; sr < cell.rowSpan; ++sr) {
                    totalHeight += m_spacing + m_rowHeights[r + sr];
                }
                
                // Position widget with padding
                ImGui::SetCursorPos(ImVec2(currentX + m_padding, currentY + m_padding));
                
                // Create child window for widget
                ImGui::BeginChild(("GridCell_" + std::to_string(r) + "_" + std::to_string(c)).c_str(),
                                ImVec2(totalWidth - 2 * m_padding, totalHeight - 2 * m_padding),
                                false);
                cell.widget->render();
                ImGui::EndChild();
            }
            
            currentX += colWidth + m_spacing;
        }
        currentY += rowHeight + m_spacing;
    }
    
    ImGui::EndGroup();
}

void GridLayout::calculateLayout(float availableWidth, float availableHeight) {
    // First pass: determine minimum sizes
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            const auto& cell = m_grid[r][c];
            if (cell.widget && 
                (r == 0 || m_grid[r-1][c].widget != cell.widget) &&
                (c == 0 || m_grid[r][c-1].widget != cell.widget)) {
                
                // Get widget's preferred size
                auto size = cell.widget->getPreferredSize();
                
                // Distribute size across spanned cells
                float cellWidth = (size.x + 2 * m_padding) / cell.colSpan;
                float cellHeight = (size.y + 2 * m_padding) / cell.rowSpan;
                
                // Update maximum sizes for affected rows and columns
                for (int sr = 0; sr < cell.rowSpan; ++sr) {
                    m_rowHeights[r + sr] = std::max(m_rowHeights[r + sr], cellHeight);
                }
                for (int sc = 0; sc < cell.colSpan; ++sc) {
                    m_colWidths[c + sc] = std::max(m_colWidths[c + sc], cellWidth);
                }
            }
        }
    }
    
    // Second pass: distribute remaining space
    float totalWidth = 0;
    float totalHeight = 0;
    
    for (float width : m_colWidths) totalWidth += width;
    for (float height : m_rowHeights) totalHeight += height;
    
    totalWidth += m_spacing * (m_cols - 1);
    totalHeight += m_spacing * (m_rows - 1);
    
    // Scale if necessary
    if (totalWidth > availableWidth) {
        float scale = availableWidth / totalWidth;
        for (float& width : m_colWidths) width *= scale;
    }
    
    if (totalHeight > availableHeight) {
        float scale = availableHeight / totalHeight;
        for (float& height : m_rowHeights) height *= scale;
    }
}

bool GridLayout::isRegionAvailable(int row, int col, int rowSpan, int colSpan) const {
    for (int r = row; r < row + rowSpan; ++r) {
        for (int c = col; c < col + colSpan; ++c) {
            if (!m_grid[r][c].isEmpty()) {
                return false;
            }
        }
    }
    return true;
}

} // namespace UI
} // namespace RebelCAD
