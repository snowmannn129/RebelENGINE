#include "graphics/ViewportLayout.h"
#include <algorithm>
#include <stdexcept>

namespace RebelCAD {
namespace Graphics {

// Initialize static memory pool
Core::MemoryPool<Viewport> ViewportLayout::s_viewportPool;

ViewportLayout::ViewportLayout(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_rows(1)
    , m_cols(1)
    , m_updating(false) {
}

ViewportLayout::~ViewportLayout() = default;

void ViewportLayout::setDimensions(int width, int height) {
    m_width = width;
    m_height = height;
    recalculateViewportDimensions();
}

void ViewportLayout::setGridSize(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
    
    m_rows = rows;
    m_cols = cols;
    recalculateViewportDimensions();
}

Viewport* ViewportLayout::addViewport(const ViewportCell& cell,
                                    ViewportProjection projection,
                                    ViewportView view) {
    if (!isValidCell(cell)) {
        throw std::invalid_argument("Invalid cell configuration");
    }
    
    if (hasOverlap(cell)) {
        throw std::invalid_argument("Cell overlaps with existing viewport");
    }
    
    // Create viewport
    auto viewport = std::make_unique<Viewport>();
    
    // Calculate viewport dimensions
    auto rect = getCellRect(cell);
    viewport->setViewport(
        static_cast<int>(rect.x),
        static_cast<int>(rect.y),
        static_cast<int>(rect.z),
        static_cast<int>(rect.w)
    );
    
    // Set initial view configuration
    const float aspect = rect.z / rect.w;
    if (projection == ViewportProjection::Perspective) {
        viewport->setPerspective(45.0f, aspect, 0.1f, 1000.0f);
    } else {
        // For orthographic, maintain consistent scale regardless of aspect ratio
        const float scale = 10.0f;
        viewport->setPerspective(0.0f, aspect, -scale, scale);
    }
    
    // Set initial camera position based on view
    glm::vec3 eye(0.0f);
    glm::vec3 target(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    
    const float distance = 10.0f;
    switch (view) {
        case ViewportView::Front:
            eye = glm::vec3(0.0f, 0.0f, distance);
            break;
        case ViewportView::Back:
            eye = glm::vec3(0.0f, 0.0f, -distance);
            break;
        case ViewportView::Top:
            eye = glm::vec3(0.0f, distance, 0.0f);
            up = glm::vec3(0.0f, 0.0f, -1.0f);
            break;
        case ViewportView::Bottom:
            eye = glm::vec3(0.0f, -distance, 0.0f);
            up = glm::vec3(0.0f, 0.0f, 1.0f);
            break;
        case ViewportView::Left:
            eye = glm::vec3(-distance, 0.0f, 0.0f);
            break;
        case ViewportView::Right:
            eye = glm::vec3(distance, 0.0f, 0.0f);
            break;
        case ViewportView::Isometric:
            eye = glm::vec3(distance) * glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
            break;
        default:
            break;
    }
    
    viewport->setView(eye, target, up);
    
    // Store viewport
    Viewport* ptr = viewport.get();
    m_viewports.push_back(ViewportEntry{std::move(viewport), cell});
    
    return ptr;
}

void ViewportLayout::removeViewport(Viewport* viewport) {
    auto it = std::find_if(m_viewports.begin(), m_viewports.end(),
        [viewport](const ViewportEntry& entry) {
            return entry.viewport.get() == viewport;
        });
    
    if (it != m_viewports.end()) {
        m_viewports.erase(it);
    }
}

Viewport* ViewportLayout::getViewport(int row, int col) const {
    auto it = std::find_if(m_viewports.begin(), m_viewports.end(),
        [row, col](const ViewportEntry& entry) {
            return entry.cell.row == row && entry.cell.col == col;
        });
    
    return it != m_viewports.end() ? it->viewport.get() : nullptr;
}

void ViewportLayout::applyPreset(const ViewportLayoutPreset& preset) {
    // Clear existing viewports
    m_viewports.clear();
    
    // Set grid size
    setGridSize(preset.rows, preset.cols);
    
    // Create viewports
    for (size_t i = 0; i < preset.cells.size(); ++i) {
        ViewportProjection projection = ViewportProjection::Perspective;
        ViewportView view = ViewportView::Isometric;
        
        if (i < preset.projections.size()) {
            projection = preset.projections[i];
        }
        if (i < preset.views.size()) {
            view = preset.views[i];
        }
        
        addViewport(preset.cells[i], projection, view);
    }
}

ViewportLayoutPreset ViewportLayout::saveAsPreset(const std::string& name) const {
    ViewportLayoutPreset preset(name, m_rows, m_cols);
    
    for (const auto& entry : m_viewports) {
        preset.cells.push_back(entry.cell);
        // TODO: Store projection and view from viewport
        preset.projections.push_back(ViewportProjection::Perspective);
        preset.views.push_back(ViewportView::Isometric);
    }
    
    return preset;
}

glm::vec4 ViewportLayout::getCellRect(const ViewportCell& cell) const {
    // Calculate cell dimensions
    const float cellWidth = static_cast<float>(m_width) / m_cols;
    const float cellHeight = static_cast<float>(m_height) / m_rows;
    
    // Calculate viewport position and size
    const float x = cell.col * cellWidth;
    const float y = (m_rows - cell.row - cell.rowSpan) * cellHeight;
    const float width = cell.colSpan * cellWidth;
    const float height = cell.rowSpan * cellHeight;
    
    return glm::vec4(x, y, width, height);
}

void ViewportLayout::updateAll() {
    if (m_updating.exchange(true)) {
        return; // Already updating
    }
    
    recalculateViewportDimensions();
    
    for (auto& entry : m_viewports) {
        // Trigger viewport update
        // TODO: Add viewport update mechanism
    }
    
    m_updating = false;
}

void ViewportLayout::synchronizeViewports(const std::vector<Viewport*>& viewports, bool sync) {
    // TODO: Implement viewport synchronization
}

void ViewportLayout::recalculateViewportDimensions() {
    for (auto& entry : m_viewports) {
        auto rect = getCellRect(entry.cell);
        entry.viewport->setViewport(
            static_cast<int>(rect.x),
            static_cast<int>(rect.y),
            static_cast<int>(rect.z),
            static_cast<int>(rect.w)
        );
    }
}

bool ViewportLayout::isValidCell(const ViewportCell& cell) const {
    return cell.row >= 0 && cell.row < m_rows &&
           cell.col >= 0 && cell.col < m_cols &&
           cell.rowSpan > 0 && cell.row + cell.rowSpan <= m_rows &&
           cell.colSpan > 0 && cell.col + cell.colSpan <= m_cols;
}

bool ViewportLayout::hasOverlap(const ViewportCell& cell) const {
    for (const auto& entry : m_viewports) {
        const auto& existing = entry.cell;
        
        // Check for overlap
        bool overlapX = (cell.col < existing.col + existing.colSpan) &&
                       (cell.col + cell.colSpan > existing.col);
        bool overlapY = (cell.row < existing.row + existing.rowSpan) &&
                       (cell.row + cell.rowSpan > existing.row);
        
        if (overlapX && overlapY) {
            return true;
        }
    }
    
    return false;
}

} // namespace Graphics
} // namespace RebelCAD
