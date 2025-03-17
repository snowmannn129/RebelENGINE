#pragma once

#include "ui/Widget.h"
#include <string>
#include <functional>

namespace RebelCAD {
namespace UI {

/**
 * @brief A clickable button widget
 * 
 * Features:
 * - Text label
 * - Click handling
 * - Hover effects
 * - Disabled state
 * - Custom styling
 */
class Button : public Widget {
public:
    /**
     * @brief Construct a new Button
     * @param id Unique identifier
     * @param label Button text
     */
    Button(const std::string& id, const std::string& label);
    virtual ~Button() = default;

    // Render implementation
    void render() override;

    // Label management
    void setLabel(const std::string& label);
    const std::string& getLabel() const;

    // Click handling
    using ClickCallback = std::function<void()>;
    void setOnClick(ClickCallback callback);

protected:
    // Event handlers
    void onMouseEnter() override;
    void onMouseLeave() override;
    void onMouseDown(int button) override;
    void onMouseUp(int button) override;
    void onClick() override;
    void onStyleChanged() override;

    // Layout calculation
    ImVec2 calculatePreferredSize() const override;

    // Friend test class
    friend class ButtonTest;

private:
    std::string m_Label;
    ClickCallback m_OnClick;
    bool m_IsHovered;
    bool m_IsPressed;

    // Style caching
    ImVec4 m_NormalColor;
    ImVec4 m_HoverColor;
    ImVec4 m_PressedColor;
    ImVec4 m_DisabledColor;
    ImVec4 m_TextColor;
    float m_Rounding;
    ImVec2 m_Padding;
    float m_BorderSize;
    ImVec4 m_BorderColor;

    void updateStyleCache();
    void applyCurrentStyle();
};

} // namespace UI
} // namespace RebelCAD
