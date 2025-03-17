#pragma once

#include "ui/Widget.h"
#include "core/EventBus.h"
#include <string>
#include <vector>
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace RebelCAD::UI {

// Event emitted when dropdown selection changes
struct DropdownSelectionChangedEvent {
    std::string selectedItem;
    int selectedIndex;
};

// Forward declaration for friend class
class DropdownTest;

class Dropdown : public Widget {
public:
    friend class DropdownTest;
    explicit Dropdown(const std::string& id);
    virtual ~Dropdown() override = default;

    // Item management
    void setItems(const std::vector<std::string>& items);
    const std::vector<std::string>& getItems() const;
    void clearItems();

    // Selection management
    void setSelectedIndex(int index);
    int getSelectedIndex() const;
    void setSelectedItem(const std::string& item);
    std::string getSelectedItem() const;

    // State management
    void setExpanded(bool expanded);
    bool isExpanded() const;

    // Input handling
    void handleKeyPress(ImGuiKey key);

    // Widget interface implementation
    void render() override;
    void update() override;

    // Event handlers
    void onKeyDown(int key, int mods) override;
    void onMouseDown(int button) override;
    void onClick() override;

private:
    std::vector<std::string> m_items;
    int m_selectedIndex;
    bool m_expanded;
    
    // Helper methods
    void validateAndUpdateIndex(int index);
    void emitSelectionChangedEvent();
    void handleKeyboardNavigation(ImGuiKey key);
    ImVec2 calculatePreferredSize() const override;
};

} // namespace RebelCAD::UI
