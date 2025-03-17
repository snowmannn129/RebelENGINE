#include "ui/widgets/Checkbox.h"
#include "imgui.h"

namespace RebelCAD {
namespace UI {

Checkbox::Checkbox(const std::string& id, const std::string& label, bool initialState)
    : Widget(id)
    , m_label(label)
    , m_isChecked(initialState)
    , m_isHovered(false)
    , m_isFocused(false)
    , m_onCheckedChanged(nullptr)
{
    setFocusable(true);
}

bool Checkbox::isChecked() const {
    return m_isChecked;
}

void Checkbox::setChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        if (m_onCheckedChanged) {
            m_onCheckedChanged(m_isChecked);
        }
        updateStyle();
    }
}

void Checkbox::onClick() {
    if (isEnabled()) {
        toggleState();
    }
}

void Checkbox::onKeyDown(int key, int mods) {
    if (isEnabled() && isFocused() && (key == 32 || key == 13)) { // Space or Enter
        toggleState();
    }
}

void Checkbox::setLabel(const std::string& label) {
    m_label = label;
}

const std::string& Checkbox::getLabel() const {
    return m_label;
}

void Checkbox::render() {
    if (!isVisible()) {
        return;
    }

    ImGui::PushID(getId().c_str());
    
    // Apply style
    pushStyle();
    
    // Set position
    ImVec2 pos = getPosition();
    ImGui::SetCursorPos(pos);
    
    // Render checkbox with label
    bool checked = m_isChecked;
    if (ImGui::Checkbox(m_label.c_str(), &checked)) {
        setChecked(checked);
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

void Checkbox::update() {
    Widget::update();
    updateStyle();
}

ImVec2 Checkbox::calculatePreferredSize() const {
    ImVec2 size;
    
    // Get style values
    float checkboxSize = 20.0f; // Default size
    float padding = 5.0f; // Default padding
    
    auto style = getStyle();
    if (!style.empty() && style.contains("checkbox")) {
        auto& checkboxStyle = style["checkbox"];
        if (checkboxStyle.contains("size")) {
            checkboxSize = checkboxStyle["size"];
        }
        if (checkboxStyle.contains("padding")) {
            padding = checkboxStyle["padding"];
        }
    }
    
    // Calculate text size
    ImVec2 textSize = ImGui::CalcTextSize(m_label.c_str());
    
    // Total width = checkbox + padding + text
    size.x = checkboxSize + padding + textSize.x;
    
    // Height is max of checkbox height and text height
    size.y = std::max(checkboxSize, textSize.y);
    
    return size;
}

void Checkbox::setOnCheckedChanged(CheckedChangedSignal callback) {
    m_onCheckedChanged = callback;
}

void Checkbox::toggleState() {
    setChecked(!m_isChecked);
}

void Checkbox::updateStyle() {
    // Update visual style based on state
    nlohmann::json style = {
        {"checkbox", {
            {"size", 20},
            {"padding", 5},
            {"borderWidth", 1},
            {"backgroundColor", m_isChecked ? "#4A90E2" : "#FFFFFF"},
            {"borderColor", m_isHovered ? "#4A90E2" : "#CCCCCC"},
            {"checkColor", "#FFFFFF"}
        }}
    };
    
    setStyle(style);
}

} // namespace UI
} // namespace RebelCAD
