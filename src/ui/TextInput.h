#pragma once

#include "ui/Widget.h"
#include <string>
#include <functional>

namespace RebelCAD {
namespace UI {

/**
 * @brief A text input widget
 * 
 * Features:
 * - Text input handling
 * - Selection management
 * - Cursor positioning
 * - Input validation
 * - Password mode
 * - Placeholder text
 */
class TextInput : public Widget {
public:
    /**
     * @brief Construct a new Text Input
     * @param id Unique identifier
     * @param placeholder Placeholder text
     */
    TextInput(const std::string& id, const std::string& placeholder = "");
    virtual ~TextInput() = default;

    // Render implementation
    void render() override;

    // Text management
    void setText(const std::string& text);
    const std::string& getText() const;
    void setPlaceholder(const std::string& placeholder);
    const std::string& getPlaceholder() const;

    // Selection management
    void setSelection(size_t start, size_t end);
    void getSelection(size_t& start, size_t& end) const;
    void selectAll();
    void clearSelection();
    bool hasSelection() const;

    // Cursor management
    void setCursorPosition(size_t position);
    size_t getCursorPosition() const;

    // Input validation
    using ValidationCallback = std::function<bool(const std::string&)>;
    void setValidator(ValidationCallback validator);
    void clearValidator();

    // Password mode
    void setPasswordMode(bool enabled);
    bool isPasswordMode() const;

    // Input handling
    using TextChangedCallback = std::function<void(const std::string&)>;
    void setOnTextChanged(TextChangedCallback callback);
    using SubmitCallback = std::function<void(const std::string&)>;
    void setOnSubmit(SubmitCallback callback);

protected:
    // Event handlers
    void onKeyDown(int key, int mods) override;
    void onKeyUp(int key, int mods) override;
    void onChar(unsigned int c) override;
    void onFocus() override;
    void onBlur() override;
    void onMouseDown(int button) override;
    void onMouseUp(int button) override;
    void onMouseMove(const ImVec2& position) override;
    void onStyleChanged() override;

    // Layout calculation
    ImVec2 calculatePreferredSize() const override;

    // Friend test classes
    friend class TextInputTest;
    friend class ::RebelCAD::UI::TextInputTest;

private:
    std::string m_Text;
    std::string m_Placeholder;
    size_t m_CursorPosition;
    size_t m_SelectionStart;
    size_t m_SelectionEnd;
    bool m_PasswordMode;
    bool m_IsDragging;

    // Callbacks
    ValidationCallback m_Validator;
    TextChangedCallback m_OnTextChanged;
    SubmitCallback m_OnSubmit;

    // Style caching
    ImVec4 m_TextColor;
    ImVec4 m_PlaceholderColor;
    ImVec4 m_SelectionColor;
    ImVec4 m_CursorColor;
    ImVec4 m_BackgroundColor;
    ImVec4 m_BorderColor;
    float m_BorderSize;
    float m_Rounding;
    ImVec2 m_Padding;

    // Helper methods
    void updateStyleCache();
    void applyCurrentStyle();
    void handleTextInput(const std::string& input);
    void handleDelete(bool forward);
    void handleCopy();
    void handleCut();
    void handlePaste();
    void updateCursorFromMouse(const ImVec2& mousePos);
    size_t getCharIndexFromPosition(const ImVec2& position) const;
    ImVec2 getPositionFromCharIndex(size_t index) const;
    std::string getSelectedText() const;
    void replaceSelection(const std::string& text);
    bool validateInput(const std::string& text) const;
    std::string getDisplayText() const;
};

} // namespace UI
} // namespace RebelCAD
