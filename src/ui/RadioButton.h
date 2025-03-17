#pragma once

#include "ui/Widget.h"
#include "ui/StyleManager.h"
#include "core/EventBus.h"
#include "imgui.h"
#include <string>
#include <vector>
#include <memory>

namespace RebelCAD {
namespace UI {

class RadioGroup;

class RadioButton : public Widget {
public:
    // Constructor
    explicit RadioButton(const std::string& id, const std::string& label = "", bool initialState = false);
    
    // Destructor
    ~RadioButton() override = default;

    // State management
    bool isSelected() const;
    void setSelected(bool selected);
    
    // Group management
    void setGroup(std::shared_ptr<RadioGroup> group);
    std::shared_ptr<RadioGroup> getGroup() const;
    
    // Event handlers
    void onClick() override;
    void onKeyDown(int key, int mods) override;
    
    // Accessibility
    void setLabel(const std::string& label);
    const std::string& getLabel() const;
    
    // Widget interface implementation
    void render() override;
    void update() override;
    ImVec2 calculatePreferredSize() const override;

    // Event signals
    using SelectionChangedSignal = std::function<void(bool)>;
    void setOnSelectionChanged(SelectionChangedSignal callback);

private:
    std::string m_label;
    bool m_isSelected;
    bool m_isHovered;
    bool m_isFocused;
    std::weak_ptr<RadioGroup> m_group;
    SelectionChangedSignal m_onSelectionChanged;

    // Internal helpers
    void updateStyle();
    void notifyGroup();
};

class RadioGroup : public std::enable_shared_from_this<RadioGroup> {
public:
    // Constructor
    explicit RadioGroup(const std::string& id);
    
    // Group management
    void addButton(RadioButton* button);
    void removeButton(RadioButton* button);
    void selectButton(RadioButton* button);
    RadioButton* getSelectedButton() const;
    
    // Accessors
    const std::string& getId() const;
    size_t getButtonCount() const;

private:
    std::string m_id;
    std::vector<RadioButton*> m_buttons;
    RadioButton* m_selectedButton;
};

} // namespace UI
} // namespace RebelCAD
