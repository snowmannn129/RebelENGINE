#include "ui/widgets/TextInput.h"
#include "core/Log.h"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace RebelCAD {
namespace UI {

TextInput::TextInput(const std::string& id, const std::string& placeholder)
    : Widget(id)
    , m_Text("")
    , m_Placeholder(placeholder)
    , m_CursorPosition(0)
    , m_SelectionStart(0)
    , m_SelectionEnd(0)
    , m_PasswordMode(false)
    , m_IsDragging(false)
    , m_TextColor(0.0f, 0.0f, 0.0f, 1.0f)
    , m_PlaceholderColor(0.5f, 0.5f, 0.5f, 1.0f)
    , m_SelectionColor(0.2f, 0.4f, 0.9f, 0.3f)
    , m_CursorColor(0.0f, 0.0f, 0.0f, 1.0f)
    , m_BackgroundColor(1.0f, 1.0f, 1.0f, 1.0f)
    , m_BorderColor(0.7f, 0.7f, 0.7f, 1.0f)
    , m_BorderSize(1.0f)
    , m_Rounding(3.0f)
    , m_Padding(5.0f, 5.0f)
{
    setFocusable(true);
    updateStyleCache();
}

void TextInput::render() {
    if (!isVisible()) return;

    // Push style variables
    pushStyle();
    applyCurrentStyle();

    // Calculate position and size
    ImVec2 pos = getPosition();
    ImVec2 size = getSize();
    if (size.x == 0 || size.y == 0) {
        size = calculatePreferredSize();
    }

    // Set position
    ImGui::SetCursorPos(pos);

    // Create unique ID
    ImGui::PushID(getId().c_str());

    // Prepare input flags
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackAlways;
    if (m_PasswordMode) {
        flags |= ImGuiInputTextFlags_Password;
    }

    // Render input field
    std::string displayText = getDisplayText();
    char buffer[1024];
    strncpy(buffer, displayText.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    bool changed = ImGui::InputText("##input", buffer, sizeof(buffer), flags);

    // Update state if text changed
    if (changed && validateInput(buffer)) {
        m_Text = buffer;
        if (m_OnTextChanged) {
            m_OnTextChanged(m_Text);
        }
    }

    // Handle focus
    if (ImGui::IsItemFocused() && !isFocused()) {
        focus();
    } else if (!ImGui::IsItemFocused() && isFocused()) {
        blur();
    }

    // Handle submit on Enter
    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        if (m_OnSubmit) {
            m_OnSubmit(m_Text);
        }
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

void TextInput::setText(const std::string& text) {
    if (validateInput(text)) {
        m_Text = text;
        m_CursorPosition = text.length();
        clearSelection();
        if (m_OnTextChanged) {
            m_OnTextChanged(m_Text);
        }
    }
}

const std::string& TextInput::getText() const {
    return m_Text;
}

void TextInput::setPlaceholder(const std::string& placeholder) {
    m_Placeholder = placeholder;
}

const std::string& TextInput::getPlaceholder() const {
    return m_Placeholder;
}

void TextInput::setSelection(size_t start, size_t end) {
    m_SelectionStart = std::min(start, m_Text.length());
    m_SelectionEnd = std::min(end, m_Text.length());
    m_CursorPosition = m_SelectionEnd;
}

void TextInput::getSelection(size_t& start, size_t& end) const {
    start = m_SelectionStart;
    end = m_SelectionEnd;
}

void TextInput::selectAll() {
    m_SelectionStart = 0;
    m_SelectionEnd = m_Text.length();
    m_CursorPosition = m_SelectionEnd;
}

void TextInput::clearSelection() {
    m_SelectionStart = m_CursorPosition;
    m_SelectionEnd = m_CursorPosition;
}

bool TextInput::hasSelection() const {
    return m_SelectionStart != m_SelectionEnd;
}

void TextInput::setCursorPosition(size_t position) {
    m_CursorPosition = std::min(position, m_Text.length());
    clearSelection();
}

size_t TextInput::getCursorPosition() const {
    return m_CursorPosition;
}

void TextInput::setValidator(ValidationCallback validator) {
    m_Validator = validator;
}

void TextInput::clearValidator() {
    m_Validator = nullptr;
}

void TextInput::setPasswordMode(bool enabled) {
    m_PasswordMode = enabled;
}

bool TextInput::isPasswordMode() const {
    return m_PasswordMode;
}

void TextInput::setOnTextChanged(TextChangedCallback callback) {
    m_OnTextChanged = callback;
}

void TextInput::setOnSubmit(SubmitCallback callback) {
    m_OnSubmit = callback;
}

void TextInput::onKeyDown(int key, int mods) {
    Widget::onKeyDown(key, mods);

    if (!isEnabled()) return;

    bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
    bool shift = (mods & GLFW_MOD_SHIFT) != 0;

    switch (key) {
        case GLFW_KEY_LEFT:
            if (ctrl) {
                // Move to previous word
                size_t pos = m_CursorPosition;
                while (pos > 0 && std::isspace(m_Text[pos - 1])) pos--;
                while (pos > 0 && !std::isspace(m_Text[pos - 1])) pos--;
                m_CursorPosition = pos;
            } else {
                // Move one character left
                if (m_CursorPosition > 0) m_CursorPosition--;
            }
            if (!shift) clearSelection();
            else setSelection(m_SelectionStart, m_CursorPosition);
            break;

        case GLFW_KEY_RIGHT:
            if (ctrl) {
                // Move to next word
                size_t pos = m_CursorPosition;
                while (pos < m_Text.length() && !std::isspace(m_Text[pos])) pos++;
                while (pos < m_Text.length() && std::isspace(m_Text[pos])) pos++;
                m_CursorPosition = pos;
            } else {
                // Move one character right
                if (m_CursorPosition < m_Text.length()) m_CursorPosition++;
            }
            if (!shift) clearSelection();
            else setSelection(m_SelectionStart, m_CursorPosition);
            break;

        case GLFW_KEY_HOME:
            m_CursorPosition = 0;
            if (!shift) clearSelection();
            else setSelection(m_SelectionStart, m_CursorPosition);
            break;

        case GLFW_KEY_END:
            m_CursorPosition = m_Text.length();
            if (!shift) clearSelection();
            else setSelection(m_SelectionStart, m_CursorPosition);
            break;

        case GLFW_KEY_DELETE:
            handleDelete(true);
            break;

        case GLFW_KEY_BACKSPACE:
            handleDelete(false);
            break;

        case GLFW_KEY_C:
            if (ctrl) handleCopy();
            break;

        case GLFW_KEY_X:
            if (ctrl) handleCut();
            break;

        case GLFW_KEY_V:
            if (ctrl) handlePaste();
            break;

        case GLFW_KEY_A:
            if (ctrl) selectAll();
            break;
    }
}

void TextInput::onKeyUp(int key, int mods) {
    Widget::onKeyUp(key, mods);
}

void TextInput::onChar(unsigned int c) {
    Widget::onChar(c);

    if (!isEnabled()) return;

    // Handle printable characters
    if (c >= 32) {
        std::string input(1, static_cast<char>(c));
        handleTextInput(input);
    }
}

void TextInput::onFocus() {
    Widget::onFocus();
}

void TextInput::onBlur() {
    Widget::onBlur();
    clearSelection();
}

void TextInput::onMouseDown(int button) {
    Widget::onMouseDown(button);

    if (button == 0) { // Left button
        m_IsDragging = true;
        ImVec2 mousePos = ImGui::GetMousePos();
        updateCursorFromMouse(mousePos);
        clearSelection();
    }
}

void TextInput::onMouseUp(int button) {
    Widget::onMouseUp(button);

    if (button == 0) { // Left button
        m_IsDragging = false;
    }
}

void TextInput::onMouseMove(const ImVec2& position) {
    Widget::onMouseMove(position);

    if (m_IsDragging) {
        updateCursorFromMouse(position);
        setSelection(m_SelectionStart, m_CursorPosition);
    }
}

void TextInput::onStyleChanged() {
    Widget::onStyleChanged();
    updateStyleCache();
}

ImVec2 TextInput::calculatePreferredSize() const {
    // Calculate text size
    ImVec2 textSize = ImGui::CalcTextSize(getDisplayText().c_str());

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

void TextInput::updateStyleCache() {
    auto style = getStyle();

    // Colors
    if (style.contains("textColor")) {
        auto color = style["textColor"];
        m_TextColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("placeholderColor")) {
        auto color = style["placeholderColor"];
        m_PlaceholderColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("selectionColor")) {
        auto color = style["selectionColor"];
        m_SelectionColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("cursorColor")) {
        auto color = style["cursorColor"];
        m_CursorColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    if (style.contains("backgroundColor")) {
        auto color = style["backgroundColor"];
        m_BackgroundColor = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
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

    // Other properties
    m_BorderSize = style.value("borderSize", 1.0f);
    m_Rounding = style.value("rounding", 3.0f);

    if (style.contains("padding")) {
        auto padding = style["padding"];
        m_Padding = ImVec2(padding[0].get<float>(), padding[1].get<float>());
    }
}

void TextInput::applyCurrentStyle() {
    ImGui::PushStyleColor(ImGuiCol_Text, m_TextColor);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, m_PlaceholderColor);
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, m_SelectionColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, m_BackgroundColor);
    ImGui::PushStyleColor(ImGuiCol_Border, m_BorderColor);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_Rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, m_Padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, m_BorderSize);

    // Pop these in reverse order in render()
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);
}

void TextInput::handleTextInput(const std::string& input) {
    std::string newText = m_Text;
    if (hasSelection()) {
        // Replace selection with input
        size_t start = std::min(m_SelectionStart, m_SelectionEnd);
        size_t end = std::max(m_SelectionStart, m_SelectionEnd);
        newText.replace(start, end - start, input);
        m_CursorPosition = start + input.length();
    } else {
        // Insert at cursor position
        newText.insert(m_CursorPosition, input);
        m_CursorPosition += input.length();
    }

    if (validateInput(newText)) {
        m_Text = newText;
        clearSelection();
        if (m_OnTextChanged) {
            m_OnTextChanged(m_Text);
        }
    }
}

void TextInput::handleDelete(bool forward) {
    if (hasSelection()) {
        // Delete selection
        size_t start = std::min(m_SelectionStart, m_SelectionEnd);
        size_t end = std::max(m_SelectionStart, m_SelectionEnd);
        m_Text.erase(start, end - start);
        m_CursorPosition = start;
        clearSelection();
    } else if (forward) {
        // Delete character after cursor
        if (m_CursorPosition < m_Text.length()) {
            m_Text.erase(m_CursorPosition, 1);
        }
    } else {
        // Delete character before cursor
        if (m_CursorPosition > 0) {
            m_Text.erase(--m_CursorPosition, 1);
        }
    }

    if (m_OnTextChanged) {
        m_OnTextChanged(m_Text);
    }
}

void TextInput::handleCopy() {
    if (hasSelection() && !m_PasswordMode) {
        ImGui::SetClipboardText(getSelectedText().c_str());
    }
}

void TextInput::handleCut() {
    if (hasSelection() && !m_PasswordMode) {
        ImGui::SetClipboardText(getSelectedText().c_str());
        handleDelete(true);
    }
}

void TextInput::handlePaste() {
    const char* clipboard = ImGui::GetClipboardText();
    if (clipboard) {
        handleTextInput(clipboard);
    }
}

void TextInput::updateCursorFromMouse(const ImVec2& mousePos) {
    m_CursorPosition = getCharIndexFromPosition(mousePos);
}

size_t TextInput::getCharIndexFromPosition(const ImVec2& position) const {
    // Convert screen position to local position
    ImVec2 localPos = ImVec2(
        position.x - getPosition().x - m_Padding.x,
        position.y - getPosition().y - m_Padding.y
    );

    // Find closest character
    std::string displayText = getDisplayText();
    float bestDistance = FLT_MAX;
    size_t bestIndex = 0;

    for (size_t i = 0; i <= displayText.length(); ++i) {
        ImVec2 charPos = getPositionFromCharIndex(i);
        float distance = std::abs(charPos.x - localPos.x);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return bestIndex;
}

ImVec2 TextInput::getPositionFromCharIndex(size_t index) const {
    std::string displayText = getDisplayText();
    if (index > displayText.length()) {
        index = displayText.length();
    }

    std::string subText = displayText.substr(0, index);
    ImVec2 textSize = ImGui::CalcTextSize(subText.c_str());
    return textSize;
}

std::string TextInput::getSelectedText() const {
    if (!hasSelection()) return "";

    size_t start = std::min(m_SelectionStart, m_SelectionEnd);
    size_t end = std::max(m_SelectionStart, m_SelectionEnd);
    return m_Text.substr(start, end - start);
}

void TextInput::replaceSelection(const std::string& text) {
    if (!hasSelection()) return;

    size_t start = std::min(m_SelectionStart, m_SelectionEnd);
    size_t end = std::max(m_SelectionStart, m_SelectionEnd);
    
    std::string newText = m_Text;
    newText.replace(start, end - start, text);
    
    if (validateInput(newText)) {
        m_Text = newText;
        m_CursorPosition = start + text.length();
        clearSelection();
        if (m_OnTextChanged) {
            m_OnTextChanged(m_Text);
        }
    }
}

bool TextInput::validateInput(const std::string& text) const {
    return !m_Validator || m_Validator(text);
}

std::string TextInput::getDisplayText() const {
    if (m_Text.empty() && !isFocused()) {
        return m_Placeholder;
    }
    if (m_PasswordMode) {
        return std::string(m_Text.length(), '*');
    }
    return m_Text;
}

} // namespace UI
} // namespace RebelCAD
