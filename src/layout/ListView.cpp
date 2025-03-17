#include "ui/widgets/ListView.h"
#include <stdexcept>
#include <algorithm>

namespace RebelCAD {
namespace UI {

ListView::ListView(const std::string& id)
    : Widget(id) {
}

ListView::ListView(const std::string& id, const std::vector<std::string>& items)
    : Widget(id), items_(items) {
}

ListView::~ListView() {
    if (selectionChangedSubscription_ != 0) {
        getEventBus().unsubscribe(selectionChangedSubscription_);
    }
}

void ListView::addItem(const std::string& item) {
    items_.push_back(item);
}

void ListView::addItems(const std::vector<std::string>& items) {
    items_.reserve(items_.size() + items.size());
    items_.insert(items_.end(), items.begin(), items.end());
}

void ListView::removeItem(size_t index) {
    validateIndex(index);
    items_.erase(items_.begin() + index);
    
    // Efficiently update selection indices
    auto it = selectedIndices_.begin();
    while (it != selectedIndices_.end()) {
        if (*it == index) {
            it = selectedIndices_.erase(it);
        } else if (*it > index) {
            size_t newIndex = *it - 1;
            it = selectedIndices_.erase(it);
            selectedIndices_.insert(newIndex);
        } else {
            ++it;
        }
    }
    notifySelectionChanged();
}

void ListView::clearItems() {
    items_.clear();
    clearSelection();
}

void ListView::setSelectionMode(SelectionMode mode) {
    selectionMode_ = mode;
    if (mode == SelectionMode::Single && selectedIndices_.size() > 1) {
        // Keep only the first selected item
        auto firstSelected = *selectedIndices_.begin();
        selectedIndices_.clear();
        selectedIndices_.insert(firstSelected);
        notifySelectionChanged();
    }
}

void ListView::setSelectedIndex(size_t index) {
    validateIndex(index);
    selectedIndices_.clear();
    selectedIndices_.insert(index);
    notifySelectionChanged();
}

void ListView::addToSelection(size_t index) {
    validateIndex(index);
    if (selectionMode_ == SelectionMode::Single) {
        setSelectedIndex(index);
    } else {
        selectedIndices_.insert(index);
        notifySelectionChanged();
    }
}

void ListView::removeFromSelection(size_t index) {
    validateIndex(index);
    if (selectedIndices_.erase(index) > 0) {
        notifySelectionChanged();
    }
}

void ListView::clearSelection() {
    if (!selectedIndices_.empty()) {
        selectedIndices_.clear();
        notifySelectionChanged();
    }
}

void ListView::onSelectionChanged(std::function<void(const std::set<size_t>&)> callback) {
    selectionChangedSubscription_ = getEventBus().subscribe<SelectionChangedEvent>(
        [callback](const SelectionChangedEvent& event, const Core::EventMetadata&) {
            callback(event.selectedIndices);
        }
    );
}

void ListView::handleKeyPress(int keyCode) {
    if (items_.empty() || !hasFocus_) return;

    size_t currentIndex = selectedIndices_.empty() ? 0 : *selectedIndices_.begin();
    
    switch (keyCode) {
        case ImGuiKey_UpArrow:
            if (currentIndex > 0) {
                setSelectedIndex(currentIndex - 1);
            }
            break;
            
        case ImGuiKey_DownArrow:
            if (currentIndex < items_.size() - 1) {
                setSelectedIndex(currentIndex + 1);
            }
            break;
            
        case ImGuiKey_Home:
            if (!items_.empty()) {
                setSelectedIndex(0);
            }
            break;
            
        case ImGuiKey_End:
            if (!items_.empty()) {
                setSelectedIndex(items_.size() - 1);
            }
            break;
    }
}

void ListView::applyStyle(const nlohmann::json& style) {
    style_ = style;
    
    // Pre-calculate and cache style colors
    if (style_.contains("backgroundColor")) {
        cachedBackgroundColor_ = ImGui::ColorConvertU32ToFloat4(
            std::stoull(style_["backgroundColor"].get<std::string>().substr(1), nullptr, 16));
    }
    if (style_.contains("selectedBackgroundColor")) {
        cachedSelectedBgColor_ = ImGui::ColorConvertU32ToFloat4(
            std::stoull(style_["selectedBackgroundColor"].get<std::string>().substr(1), nullptr, 16));
    }
    if (style_.contains("selectedTextColor")) {
        cachedSelectedTextColor_ = ImGui::ColorConvertU32ToFloat4(
            std::stoull(style_["selectedTextColor"].get<std::string>().substr(1), nullptr, 16));
    }
    
    // Cache other style values
    itemHeight_ = style_.value("itemHeight", 20.0f);
}

ImVec2 ListView::calculatePreferredSize() const {
    float width = 200.0f; // Default minimum width
    float height = items_.size() * itemHeight_;
    
    // Add padding
    width += 20.0f;
    height += 10.0f;
    
    return ImVec2(width, height);
}

void ListView::setFocus(bool focused) {
    Widget::setFocus(focused);
}

void ListView::setScrollPosition(float position) {
    scrollPosition_ = std::max(0.0f, position);
    updateScrollBounds();
}

void ListView::render() {
    if (!isVisible_) return;

    ImGui::PushID(id_.c_str());
    
    // Apply cached style
    if (style_.contains("backgroundColor")) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, cachedBackgroundColor_);
    }

    // Calculate visible region
    float listHeight = size_.y > 0 ? size_.y : calculatePreferredSize().y;
    ImGui::BeginChild(id_.c_str(), ImVec2(size_.x, listHeight), true);
    
    // Calculate visible item range based on scroll position
    float clipperStart = scrollPosition_ / itemHeight_;
    float clipperEnd = (scrollPosition_ + listHeight) / itemHeight_ + 1;
    size_t startItem = static_cast<size_t>(std::max(0.0f, clipperStart));
    size_t endItem = static_cast<size_t>(std::min(static_cast<float>(items_.size()), clipperEnd));
    
    // Only render visible items
    for (size_t i = startItem; i < endItem; i++) {
        bool isSelected = selectedIndices_.find(i) != selectedIndices_.end();
        
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Header, cachedSelectedBgColor_);
            ImGui::PushStyleColor(ImGuiCol_Text, cachedSelectedTextColor_);
        }
        
        if (ImGui::Selectable(items_[i].c_str(), isSelected)) {
            if (ImGui::GetIO().KeyCtrl && selectionMode_ == SelectionMode::Multiple) {
                if (isSelected) {
                    removeFromSelection(i);
                } else {
                    addToSelection(i);
                }
            } else {
                setSelectedIndex(i);
            }
        }
        
        if (isSelected) {
            ImGui::PopStyleColor(2);
        }
    }
    
    ImGui::EndChild();
    
    if (style_.contains("backgroundColor")) {
        ImGui::PopStyleColor();
    }
    
    ImGui::PopID();
}

void ListView::validateIndex(size_t index) const {
    if (index >= items_.size()) {
        throw std::out_of_range("Index out of range");
    }
}

void ListView::notifySelectionChanged() {
    SelectionChangedEvent event{selectedIndices_};
    getEventBus().publish(event);
}

void ListView::updateScrollBounds() {
    float maxScroll = std::max(0.0f, items_.size() * itemHeight_ - size_.y);
    scrollPosition_ = std::min(scrollPosition_, maxScroll);
}

} // namespace UI
} // namespace RebelCAD
