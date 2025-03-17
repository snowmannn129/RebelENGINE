#include "ui/widgets/Dropdown.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <imgui_internal.h>

namespace RebelCAD::UI {

Dropdown::Dropdown(const std::string& id)
    : Widget(id)
    , m_selectedIndex(-1)
    , m_expanded(false)
{
    setFocusable(true);
}

void Dropdown::setItems(const std::vector<std::string>& items) {
    m_items = items;
    validateAndUpdateIndex(m_selectedIndex);
}

const std::vector<std::string>& Dropdown::getItems() const {
    return m_items;
}

void Dropdown::clearItems() {
    m_items.clear();
    setSelectedIndex(-1);
}

void Dropdown::setSelectedIndex(int index) {
    if (!isEnabled()) {
        return;
    }
    
    if (index != m_selectedIndex) {
        validateAndUpdateIndex(index);
        emitSelectionChangedEvent();
    }
}

int Dropdown::getSelectedIndex() const {
    return m_selectedIndex;
}

void Dropdown::setSelectedItem(const std::string& item) {
    if (!isEnabled()) {
        return;
    }

    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        int index = static_cast<int>(std::distance(m_items.begin(), it));
        setSelectedIndex(index);
    } else {
        setSelectedIndex(-1);
    }
}

std::string Dropdown::getSelectedItem() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
        return m_items[m_selectedIndex];
    }
    return "";
}

void Dropdown::setExpanded(bool expanded) {
    if (m_expanded != expanded) {
        m_expanded = expanded;
        triggerEvent("expandedStateChanged");
    }
}

bool Dropdown::isExpanded() const {
    return m_expanded;
}

void Dropdown::handleKeyPress(ImGuiKey key) {
    if (!isEnabled() || m_items.empty()) {
        return;
    }

    switch (key) {
        case ImGuiKey_UpArrow:
            if (m_selectedIndex > 0) {
                setSelectedIndex(m_selectedIndex - 1);
            } else if (m_selectedIndex == -1) {
                setSelectedIndex(static_cast<int>(m_items.size()) - 1);
            }
            break;

        case ImGuiKey_DownArrow:
            if (m_selectedIndex < static_cast<int>(m_items.size()) - 1) {
                setSelectedIndex(m_selectedIndex + 1);
            } else if (m_selectedIndex == -1) {
                setSelectedIndex(0);
            }
            break;

        case ImGuiKey_Home:
            if (!m_items.empty()) {
                setSelectedIndex(0);
            }
            break;

        case ImGuiKey_End:
            if (!m_items.empty()) {
                setSelectedIndex(static_cast<int>(m_items.size()) - 1);
            }
            break;

        case ImGuiKey_Enter:
        case ImGuiKey_Space:
            setExpanded(!m_expanded);
            break;

        case ImGuiKey_Escape:
            if (m_expanded) {
                setExpanded(false);
            }
            break;
    }
}

void Dropdown::render() {
    if (!isVisible()) {
        return;
    }

    ImGui::PushID(getId().c_str());

    // Apply style
    auto style = getStyle();
    if (style.contains("backgroundColor")) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(
            style["backgroundColor"][0],
            style["backgroundColor"][1],
            style["backgroundColor"][2],
            style["backgroundColor"][3]
        ));
    }
    if (style.contains("textColor")) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(
            style["textColor"][0],
            style["textColor"][1],
            style["textColor"][2],
            style["textColor"][3]
        ));
    }
    if (style.contains("borderColor")) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(
            style["borderColor"][0],
            style["borderColor"][1],
            style["borderColor"][2],
            style["borderColor"][3]
        ));
    }
    if (style.contains("rounding")) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style["rounding"]);
    }
    if (style.contains("padding")) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(
            style["padding"][0],
            style["padding"][1]
        ));
    }
    if (style.contains("borderSize")) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style["borderSize"]);
    }

    // Set position and size
    ImGui::SetCursorPos(ImVec2(getPosition().x, getPosition().y));
    if (getSize().x > 0 && getSize().y > 0) {
        ImGui::SetNextItemWidth(getSize().x);
    }

    // Render dropdown
    const char* preview = m_selectedIndex >= 0 ? m_items[m_selectedIndex].c_str() : "##empty";
    bool wasExpanded = m_expanded;
    if (ImGui::BeginCombo("##dropdown", preview, ImGuiComboFlags_NoArrowButton)) {
        for (size_t i = 0; i < m_items.size(); i++) {
            bool isSelected = (static_cast<int>(i) == m_selectedIndex);
            if (ImGui::Selectable(m_items[i].c_str(), isSelected)) {
                setSelectedIndex(static_cast<int>(i));
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    // Update expanded state
    bool isExpanded = ImGui::IsPopupOpen("##dropdown");
    if (wasExpanded != isExpanded) {
        setExpanded(isExpanded);
    }

    // Pop style
    if (style.contains("backgroundColor")) ImGui::PopStyleColor();
    if (style.contains("textColor")) ImGui::PopStyleColor();
    if (style.contains("borderColor")) ImGui::PopStyleColor();
    if (style.contains("rounding")) ImGui::PopStyleVar();
    if (style.contains("padding")) ImGui::PopStyleVar();
    if (style.contains("borderSize")) ImGui::PopStyleVar();

    ImGui::PopID();
}

void Dropdown::update() {
    Widget::update();
}

void Dropdown::onKeyDown(int key, int mods) {
    Widget::onKeyDown(key, mods);
    
    if (!isEnabled()) {
        return;
    }

    switch (key) {
        case GLFW_KEY_UP:
            handleKeyPress(ImGuiKey_UpArrow);
            break;
        case GLFW_KEY_DOWN:
            handleKeyPress(ImGuiKey_DownArrow);
            break;
        case GLFW_KEY_HOME:
            handleKeyPress(ImGuiKey_Home);
            break;
        case GLFW_KEY_END:
            handleKeyPress(ImGuiKey_End);
            break;
        case GLFW_KEY_ENTER:
            handleKeyPress(ImGuiKey_Enter);
            break;
        case GLFW_KEY_SPACE:
            handleKeyPress(ImGuiKey_Space);
            break;
        case GLFW_KEY_ESCAPE:
            handleKeyPress(ImGuiKey_Escape);
            break;
    }
}

void Dropdown::onMouseDown(int button) {
    Widget::onMouseDown(button);
    
    if (!isEnabled() || button != 0) { // Left button only
        return;
    }

    setExpanded(!m_expanded);
}

void Dropdown::onClick() {
    Widget::onClick();
    
    if (!isEnabled()) {
        return;
    }

    focus();
}

void Dropdown::validateAndUpdateIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_selectedIndex = index;
    } else {
        m_selectedIndex = -1;
    }
}

void Dropdown::emitSelectionChangedEvent() {
    triggerEvent("selectionChanged");
}

ImVec2 Dropdown::calculatePreferredSize() const {
    // Calculate width based on longest item
    float maxWidth = 0;
    for (const auto& item : m_items) {
        ImVec2 textSize = ImGui::CalcTextSize(item.c_str());
        maxWidth = std::max(maxWidth, textSize.x);
    }

    // Add padding and arrow space
    ImVec2 padding = ImGui::GetStyle().FramePadding;
    float arrowWidth = ImGui::GetFrameHeight();
    maxWidth += padding.x * 2 + arrowWidth;

    // Height is single line plus padding
    float height = ImGui::GetFrameHeight();

    return ImVec2(maxWidth, height);
}

} // namespace RebelCAD::UI
