#include "ui/widgets/RadioButton.h"
#include "imgui.h"

namespace RebelCAD {
namespace UI {

// RadioButton Implementation

RadioButton::RadioButton(const std::string& id, const std::string& label, bool initialState)
    : Widget(id)
    , m_label(label)
    , m_isSelected(initialState)
    , m_isHovered(false)
    , m_isFocused(false)
    , m_onSelectionChanged(nullptr)
{
    setFocusable(true);
}

bool RadioButton::isSelected() const {
    return m_isSelected;
}

void RadioButton::setSelected(bool selected) {
    if (m_isSelected != selected) {
        // If trying to deselect when we're the only selected button, ignore
        if (!selected && m_group.lock() && m_group.lock()->getSelectedButton() == this) {
            return;
        }

        m_isSelected = selected;
        
        // Notify group of selection
        if (selected && m_group.lock()) {
            notifyGroup();
        }
        
        if (m_onSelectionChanged) {
            m_onSelectionChanged(m_isSelected);
        }
        
        updateStyle();
    }
}

void RadioButton::setGroup(std::shared_ptr<RadioGroup> group) {
    // Remove from old group if exists
    if (auto oldGroup = m_group.lock()) {
        oldGroup->removeButton(this);
    }
    
    m_group = group;
    
    // Add to new group if exists
    if (group) {
        group->addButton(this);
        if (m_isSelected) {
            notifyGroup();
        }
    }
}

std::shared_ptr<RadioGroup> RadioButton::getGroup() const {
    return m_group.lock();
}

void RadioButton::onClick() {
    if (isEnabled()) {
        setSelected(true);
    }
}

void RadioButton::onKeyDown(int key, int mods) {
    if (!isEnabled() || !isFocused()) {
        return;
    }

    if (key == 32 || key == 13) { // Space or Enter
        setSelected(true);
    }
    else if (auto group = m_group.lock()) {
        // Arrow key navigation within group
        if (key == 39 || key == 40) { // Right or Down
            auto next = group->getSelectedButton();
            bool found = false;
            for (size_t i = 0; i < group->getButtonCount(); i++) {
                RadioButton* button = group->getSelectedButton();
                if (found) {
                    button->setSelected(true);
                    button->focus();
                    break;
                }
                if (button == this) {
                    found = true;
                }
            }
        }
        else if (key == 37 || key == 38) { // Left or Up
            RadioButton* prev = nullptr;
            for (size_t i = 0; i < group->getButtonCount(); i++) {
                RadioButton* button = group->getSelectedButton();
                if (button == this && prev) {
                    prev->setSelected(true);
                    prev->focus();
                    break;
                }
                prev = button;
            }
        }
    }
}

void RadioButton::setLabel(const std::string& label) {
    m_label = label;
}

const std::string& RadioButton::getLabel() const {
    return m_label;
}

void RadioButton::render() {
    if (!isVisible()) {
        return;
    }

    ImGui::PushID(getId().c_str());
    
    // Apply style
    pushStyle();
    
    // Set position
    ImVec2 pos = getPosition();
    ImGui::SetCursorPos(pos);
    
    // Render radio button with label
    bool selected = m_isSelected;
    if (ImGui::RadioButton(m_label.c_str(), &selected)) {
        setSelected(selected);
    }
    
    // Update hover state
    m_isHovered = ImGui::IsItemHovered();
    
    // Update focus state
    if (ImGui::IsItemFocused()) {
        if (!m_isFocused) {
            m_isFocused = true;
            onFocus();
        }
    } else if (m_isFocused) {
        m_isFocused = false;
        onBlur();
    }
    
    popStyle();
    ImGui::PopID();
}

void RadioButton::update() {
    Widget::update();
    updateStyle();
}

ImVec2 RadioButton::calculatePreferredSize() const {
    ImVec2 size;
    
    // Get style values
    float radioSize = 20.0f; // Default size
    float padding = 5.0f; // Default padding
    
    auto style = getStyle();
    if (!style.empty() && style.contains("radioButton")) {
        auto& radioStyle = style["radioButton"];
        if (radioStyle.contains("size")) {
            radioSize = radioStyle["size"];
        }
        if (radioStyle.contains("padding")) {
            padding = radioStyle["padding"];
        }
    }
    
    // Calculate text size
    ImVec2 textSize = ImGui::CalcTextSize(m_label.c_str());
    
    // Total width = radio + padding + text
    size.x = radioSize + padding + textSize.x;
    
    // Height is max of radio height and text height
    size.y = std::max(radioSize, textSize.y);
    
    return size;
}

void RadioButton::setOnSelectionChanged(SelectionChangedSignal callback) {
    m_onSelectionChanged = callback;
}

void RadioButton::updateStyle() {
    // Update visual style based on state
    nlohmann::json style = {
        {"radioButton", {
            {"size", 20},
            {"padding", 5},
            {"borderWidth", 1},
            {"backgroundColor", m_isSelected ? "#4A90E2" : "#FFFFFF"},
            {"borderColor", m_isHovered ? "#4A90E2" : "#CCCCCC"},
            {"dotColor", "#FFFFFF"}
        }}
    };
    
    setStyle(style);
}

void RadioButton::notifyGroup() {
    if (auto group = m_group.lock()) {
        group->selectButton(this);
    }
}

// RadioGroup Implementation

RadioGroup::RadioGroup(const std::string& id)
    : m_id(id)
    , m_selectedButton(nullptr)
{
}

void RadioGroup::addButton(RadioButton* button) {
    if (button) {
        // Remove from current position if already in group
        auto it = std::find(m_buttons.begin(), m_buttons.end(), button);
        if (it != m_buttons.end()) {
            return;
        }
        
        m_buttons.push_back(button);
        
        // If this is the first button and it's selected, update selected button
        if (m_buttons.size() == 1 && button->isSelected()) {
            m_selectedButton = button;
        }
    }
}

void RadioGroup::removeButton(RadioButton* button) {
    if (button) {
        auto it = std::find(m_buttons.begin(), m_buttons.end(), button);
        if (it != m_buttons.end()) {
            if (m_selectedButton == button) {
                m_selectedButton = nullptr;
            }
            m_buttons.erase(it);
        }
    }
}

void RadioGroup::selectButton(RadioButton* button) {
    if (!button || std::find(m_buttons.begin(), m_buttons.end(), button) == m_buttons.end()) {
        return;
    }
    
    // Deselect current button
    if (m_selectedButton && m_selectedButton != button) {
        m_selectedButton->setSelected(false);
    }
    
    m_selectedButton = button;
}

RadioButton* RadioGroup::getSelectedButton() const {
    return m_selectedButton;
}

const std::string& RadioGroup::getId() const {
    return m_id;
}

size_t RadioGroup::getButtonCount() const {
    return m_buttons.size();
}

} // namespace UI
} // namespace RebelCAD
