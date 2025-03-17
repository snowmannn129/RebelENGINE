#include "../../../include/ui/layouts/StackLayout.h"
#include <algorithm>

namespace RebelCAD {
namespace UI {

StackLayout::StackLayout(Orientation orientation)
    : m_orientation(orientation)
    , m_spacing(0.0f)
{
}

void StackLayout::addWidget(std::shared_ptr<Widget> widget) {
    if (widget) {
        m_widgets.push_back(widget);
    }
}

bool StackLayout::removeWidget(std::shared_ptr<Widget> widget) {
    auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
    if (it != m_widgets.end()) {
        m_widgets.erase(it);
        return true;
    }
    return false;
}

const std::vector<std::shared_ptr<Widget>>& StackLayout::widgets() const {
    return m_widgets;
}

void StackLayout::setSpacing(float spacing) {
    m_spacing = std::max(0.0f, spacing);
}

float StackLayout::spacing() const {
    return m_spacing;
}

void StackLayout::setOrientation(Orientation orientation) {
    m_orientation = orientation;
}

StackLayout::Orientation StackLayout::orientation() const {
    return m_orientation;
}

void StackLayout::updateLayout(float x, float y, float width, float height) {
    if (m_widgets.empty() || width < 0 || height < 0) {
        return;
    }

    // Ensure positive dimensions
    width = std::max(0.0f, width);
    height = std::max(0.0f, height);

    float currentPos = 0.0f;
    const size_t widgetCount = m_widgets.size();

    for (size_t i = 0; i < widgetCount; ++i) {
        auto& widget = m_widgets[i];
        if (!widget) continue;

        ImVec2 widgetSize = widget->getSize();
        
        if (m_orientation == Orientation::Vertical) {
            // In vertical mode, widgets take full width
            widget->setPosition(ImVec2(x, y + currentPos));
            widget->setSize(ImVec2(width, widgetSize.y));
            currentPos += widgetSize.y + m_spacing;
        } else {
            // In horizontal mode, widgets take full height
            widget->setPosition(ImVec2(x + currentPos, y));
            widget->setSize(ImVec2(widgetSize.x, height));
            currentPos += widgetSize.x + m_spacing;
        }
    }
}

} // namespace UI
} // namespace RebelCAD
