#pragma once

#include <string>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include "ui/widgets/Widget.h"
#include <core/EventBus.h>
#include "imgui.h"
#include <nlohmann/json.hpp>

namespace RebelCAD {
namespace UI {

class ListView : public Widget {
public:
    enum class SelectionMode {
        Single,
        Multiple
    };

    // Constructors
    explicit ListView(const std::string& id);
    ListView(const std::string& id, const std::vector<std::string>& items);
    virtual ~ListView();

    // Item management
    void addItem(const std::string& item);
    void addItems(const std::vector<std::string>& items);
    void removeItem(size_t index);
    void clearItems();
    const std::vector<std::string>& getItems() const { return items_; }

    // Selection management
    void setSelectionMode(SelectionMode mode);
    void setSelectedIndex(size_t index);
    void addToSelection(size_t index);
    void removeFromSelection(size_t index);
    void clearSelection();
    const std::set<size_t>& getSelectedIndices() const { return selectedIndices_; }

    // Event handling
    void onSelectionChanged(std::function<void(const std::set<size_t>&)> callback);
    void handleKeyPress(int keyCode);

    // Style
    void applyStyle(const nlohmann::json& style) override;
    const nlohmann::json& getStyle() const { return style_; }

    // Layout
    ImVec2 calculatePreferredSize() const override;

    // Focus management
    bool isFocusable() const override { return true; }
    void setFocus(bool focused) override;
    bool hasFocus() const override { return hasFocus_; }

    // Visibility
    bool isVisible() const override { return isVisible_; }
    void setVisible(bool visible) override { isVisible_ = visible; }

    // Scroll position
    void setScrollPosition(float position);
    float getScrollPosition() const { return scrollPosition_; }

    // ImGui rendering
    void render() override;

private:
    std::vector<std::string> items_;
    std::set<size_t> selectedIndices_;
    SelectionMode selectionMode_ = SelectionMode::Single;
    // Event handling
    struct SelectionChangedEvent {
        std::set<size_t> selectedIndices;
    };
    
    size_t selectionChangedSubscription_{0};
    nlohmann::json style_;
    bool hasFocus_ = false;
    bool isVisible_ = true;
    float scrollPosition_ = 0.0f;
    float itemHeight_ = 20.0f; // Default item height

    // Cached style colors for improved rendering performance
    ImVec4 cachedBackgroundColor_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 cachedSelectedBgColor_ = ImVec4(0.2f, 0.4f, 0.8f, 1.0f);
    ImVec4 cachedSelectedTextColor_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    void validateIndex(size_t index) const;
    void notifySelectionChanged();
    void updateScrollBounds();
};

} // namespace UI
} // namespace RebelCAD
