#include "ui/widgets/Button.h"
#include "core/Log.h"
#include <imgui.h>

namespace RebelCAD {
namespace UI {

Button::Button(const std::string& id, const std::string& label)
    : Widget(id)
    , m_Label(label)
    , m_IsHovered(false)
    , m_IsPressed(false)
    , m_NormalColor(0.26f, 0.59f, 0.98f, 0.40f)
    , m_HoverColor(0.26f, 0.59f, 0.98f, 1.00f)
    , m_PressedColor(0.06f, 0.53f, 0.98f, 1.00f)
    , m_DisabledColor(0.26f, 0.59f, 0.98f, 0.20f)
    , m_TextColor(1.0f, 1.0f, 1.0f, 1.0f)
    , m_Rounding(3.0f)
    , m_Padding(8.0f, 4.0f)
    , m_BorderSize(1.0f)
    , m_BorderColor(0.26f, 0.59f, 0.98f, 0.80f)
{
    setFocusable(true);
    updateStyleCache();
}

void Button::render() {
    if (!isVisible()) return;

    // Push style variables
    pushStyle();
    applyCurrentStyle();

    // Calculate button position and size
    ImVec2 pos = getPosition();
    ImVec2 size = getSize();
    if (size.x == 0 || size.y == 0) {
        size = calculatePreferredSize();
    }

    // Set button position
    ImGui::SetCursorPos(pos);

    // Create unique ID for ImGui
    ImGui::PushID(getId().c_str());

    // Render button
    bool clicked = ImGui::Button(m_Label.c_str(), size);

    // Update state
    m_IsHovered = ImGui::IsItemHovered();
    m_IsPressed = ImGui::IsItemActive();

    // Handle click
    if (clicked && isEnabled() && m_OnClick) {
        m_OnClick();
    }

    ImGui::PopID();

    // Pop style
    popStyle();

    // Update children
    for (auto& child : getChildren()) {
        if (child) {
            child->render();
        }
    }
}

void Button::setLabel(const std::string& label) {
    if (m_Label != label) {
        m_Label = label;
        invalidateLayout();
    }
}

const std::string& Button::getLabel() const {
    return m_Label;
}

void Button::setOnClick(ClickCallback callback) {
    m_OnClick = callback;
}

void Button::onMouseEnter() {
    Widget::onMouseEnter();
    m_IsHovered = true;
}

void Button::onMouseLeave() {
    Widget::onMouseLeave();
    m_IsHovered = false;
    m_IsPressed = false;
}

void Button::onMouseDown(int button) {
    Widget::onMouseDown(button);
    if (button == 0) { // Left button
        m_IsPressed = true;
    }
}

void Button::onMouseUp(int button) {
    Widget::onMouseUp(button);
    if (button == 0) { // Left button
        if (m_IsPressed && m_IsHovered && isEnabled() && m_OnClick) {
            m_OnClick();
        }
        m_IsPressed = false;
    }
}

void Button::onClick() {
    Widget::onClick();
    if (isEnabled() && m_OnClick) {
        m_OnClick();
    }
}

void Button::onStyleChanged() {
    Widget::onStyleChanged();
    updateStyleCache();
}

ImVec2 Button::calculatePreferredSize() const {
    // Calculate text size
    ImVec2 textSize = ImGui::CalcTextSize(m_Label.c_str());

    // Add padding
    ImVec2 size;
    size.x = textSize.x + m_Padding.x * 2;
    size.y = textSize.y + m_Padding.y * 2;

    // Apply min/max constraints
    ImVec2 minSize = getMinSize();
    ImVec2 maxSize = getMaxSize();
    size.x = std::max(minSize.x, std::min(maxSize.x, size.x));
    size.y = std::max(minSize.y, std::min(maxSize.y, size.y));

    return size;
}

void Button::updateStyleCache() {
    auto style = getStyle();

    // Colors
    if (style.contains("normalColor")) {
        auto color = style["normalColor"];
        m_NormalColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("hoverColor")) {
        auto color = style["hoverColor"];
        m_HoverColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("pressedColor")) {
        auto color = style["pressedColor"];
        m_PressedColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("disabledColor")) {
        auto color = style["disabledColor"];
        m_DisabledColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("textColor")) {
        auto color = style["textColor"];
        m_TextColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    // Other properties
    m_Rounding = style.value("rounding", 3.0f);
    m_BorderSize = style.value("borderSize", 1.0f);

    if (style.contains("padding")) {
        auto padding = style["padding"];
        m_Padding = ImVec2(padding[0].get<float>(), padding[1].get<float>());
    }

    if (style.contains("borderColor")) {
        auto color = style["borderColor"];
        m_BorderColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }
}

void Button::applyCurrentStyle() {
    ImVec4 bgColor;
    if (!isEnabled()) {
        bgColor = m_DisabledColor;
    } else if (m_IsPressed) {
        bgColor = m_PressedColor;
    } else if (m_IsHovered) {
        bgColor = m_HoverColor;
    } else {
        bgColor = m_NormalColor;
    }

    ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_HoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_PressedColor);
    ImGui::PushStyleColor(ImGuiCol_Text, m_TextColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, m_Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, m_BorderSize);
    ImGui::PushStyleColor(ImGuiCol_Border, m_BorderColor);

    // Pop these in reverse order in render()
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(3);
}

} // namespace UI
} // namespace RebelCAD
