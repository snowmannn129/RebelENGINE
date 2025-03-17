#include "../../include/ui/layouts/FlexLayout.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace RebelCAD {
namespace UI {

FlexLayout::FlexLayout()
    : m_direction(Direction::Row)
    , m_wrap(Wrap::NoWrap)
    , m_justifyContent(JustifyContent::Start)
    , m_alignItems(AlignItems::Start)
    , m_alignContent(AlignContent::Start)
    , m_gap(5.0f) {
}

bool FlexLayout::addWidget(std::shared_ptr<Widget> widget, 
                         const FlexItemProperties& properties) {
    if (!widget) {
        return false;
    }
    
    FlexItem item;
    item.widget = widget;
    item.properties = properties;
    m_items.push_back(item);
    return true;
}

bool FlexLayout::removeWidget(std::shared_ptr<Widget> widget) {
    auto it = std::find_if(m_items.begin(), m_items.end(),
        [&widget](const FlexItem& item) { return item.widget == widget; });
    
    if (it != m_items.end()) {
        m_items.erase(it);
        return true;
    }
    return false;
}

void FlexLayout::setDirection(Direction direction) {
    m_direction = direction;
}

void FlexLayout::setWrap(Wrap wrap) {
    m_wrap = wrap;
}

void FlexLayout::setJustifyContent(JustifyContent justify) {
    m_justifyContent = justify;
}

void FlexLayout::setAlignItems(AlignItems align) {
    m_alignItems = align;
}

void FlexLayout::setAlignContent(AlignContent align) {
    m_alignContent = align;
}

void FlexLayout::setGap(float gap) {
    m_gap = std::max(0.0f, gap);
}

bool FlexLayout::isMainAxisHorizontal() const {
    return m_direction == Direction::Row || m_direction == Direction::RowReverse;
}

void FlexLayout::update(float availableWidth, float availableHeight) {
    // Clear previous layout calculations
    m_lines.clear();
    
    if (m_items.empty()) {
        return;
    }

    const bool isHorizontal = isMainAxisHorizontal();
    const float mainAxisSize = isHorizontal ? availableWidth : availableHeight;
    const float crossAxisSize = isHorizontal ? availableHeight : availableWidth;
    
    // First pass: Calculate sizes and create lines
    std::vector<FlexItem*> currentLine;
    float currentMainSize = 0;
    float currentCrossSize = 0;
    
    for (auto& item : m_items) {
        auto size = item.widget->getPreferredSize();
        float itemMainSize = isHorizontal ? size.x : size.y;
        float itemCrossSize = isHorizontal ? size.y : size.x;
        
        // Apply flex basis if set
        if (item.properties.basis > 0) {
            itemMainSize = item.properties.basis;
        }
        
        // Check if item fits in current line
        if (m_wrap != Wrap::NoWrap && 
            !currentLine.empty() && 
            currentMainSize + m_gap + itemMainSize > mainAxisSize) {
            // Start new line
            m_lines.push_back(currentLine);
            currentLine.clear();
            currentMainSize = 0;
            currentCrossSize = 0;
        }
        
        currentLine.push_back(&item);
        currentMainSize += (currentLine.size() > 1 ? m_gap : 0) + itemMainSize;
        currentCrossSize = std::max(currentCrossSize, itemCrossSize);
        
        item.computedMainSize = itemMainSize;
        item.computedCrossSize = itemCrossSize;
    }
    
    // Add last line
    if (!currentLine.empty()) {
        m_lines.push_back(currentLine);
    }
    
    // Second pass: Distribute space in each line
    for (auto& line : m_lines) {
        float lineMainSize = std::accumulate(line.begin(), line.end(), 0.0f,
            [this](float sum, FlexItem* item) {
                return sum + item->computedMainSize + (sum > 0 ? m_gap : 0);
            });
            
        float freeSpace = mainAxisSize - lineMainSize;
        
        if (freeSpace > 0) {
            distributeSpace(line, freeSpace);
        }
        
        // Align items in line
        float lineCrossSize = (*std::max_element(line.begin(), line.end(),
            [](FlexItem* a, FlexItem* b) {
                return a->computedCrossSize < b->computedCrossSize;
            }))->computedCrossSize;
            
        alignItemsInLine(line, lineCrossSize);
    }
    
    // Third pass: Position items
    float currentY = 0;
    for (const auto& line : m_lines) {
        float currentX = 0;
        float lineHeight = (*std::max_element(line.begin(), line.end(),
            [](FlexItem* a, FlexItem* b) {
                return a->computedCrossSize < b->computedCrossSize;
            }))->computedCrossSize;
        
        for (auto item : line) {
            if (isHorizontal) {
                item->position[0] = currentX;
                item->position[1] = currentY;
            } else {
                item->position[0] = currentY;
                item->position[1] = currentX;
            }
            currentX += item->computedMainSize + m_gap;
        }
        currentY += lineHeight + m_gap;
    }
}

void FlexLayout::distributeSpace(std::vector<FlexItem*>& line, float freeSpace) {
    // Calculate total grow factor
    float totalGrow = std::accumulate(line.begin(), line.end(), 0.0f,
        [](float sum, FlexItem* item) { return sum + item->properties.grow; });
        
    if (totalGrow > 0) {
        // Distribute space according to grow factors
        for (auto item : line) {
            if (item->properties.grow > 0) {
                item->computedMainSize += (freeSpace * item->properties.grow) / totalGrow;
            }
        }
    } else {
        // Apply justify-content when no growing
        float spacing = 0;
        switch (m_justifyContent) {
            case JustifyContent::End:
                line.front()->position[0] = freeSpace;
                break;
            case JustifyContent::Center:
                line.front()->position[0] = freeSpace / 2;
                break;
            case JustifyContent::SpaceBetween:
                if (line.size() > 1) {
                    spacing = freeSpace / (line.size() - 1);
                }
                break;
            case JustifyContent::SpaceAround:
                spacing = freeSpace / line.size();
                line.front()->position[0] = spacing / 2;
                break;
            case JustifyContent::SpaceEvenly:
                spacing = freeSpace / (line.size() + 1);
                line.front()->position[0] = spacing;
                break;
            default: // Start
                break;
        }
        
        if (spacing > 0) {
            for (size_t i = 1; i < line.size(); ++i) {
                line[i]->position[0] = line[i-1]->position[0] + 
                                     line[i-1]->computedMainSize + 
                                     spacing;
            }
        }
    }
}

void FlexLayout::alignItemsInLine(std::vector<FlexItem*>& line, float lineHeight) {
    for (auto item : line) {
        float offset = 0;
        AlignItems align = item->properties.alignSelf != AlignItems::Start ? 
                          item->properties.alignSelf : m_alignItems;
                          
        switch (align) {
            case AlignItems::End:
                offset = lineHeight - item->computedCrossSize;
                break;
            case AlignItems::Center:
                offset = (lineHeight - item->computedCrossSize) / 2;
                break;
            case AlignItems::Stretch:
                item->computedCrossSize = lineHeight;
                break;
            case AlignItems::Baseline:
                // TODO: Implement baseline alignment
                break;
            default: // Start
                break;
        }
        
        if (isMainAxisHorizontal()) {
            item->position[1] += offset;
        } else {
            item->position[0] += offset;
        }
    }
}

void FlexLayout::render() {
    ImGui::BeginGroup();
    
    for (const auto& item : m_items) {
        ImGui::SetCursorPos(ImVec2(item.position[0], item.position[1]));
        
        // Create child window for widget
        ImGui::BeginChild(("FlexItem_" + std::to_string(reinterpret_cast<uintptr_t>(item.widget.get()))).c_str(),
                         ImVec2(item.computedMainSize, item.computedCrossSize),
                         false);
        item.widget->render();
        ImGui::EndChild();
    }
    
    ImGui::EndGroup();
}

} // namespace UI
} // namespace RebelCAD
