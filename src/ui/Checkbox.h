#pragma once

#include "ui/Widget.h"
#include "ui/StyleManager.h"
#include "core/EventBus.h"
#include "imgui.h"

namespace RebelCAD {
namespace UI {

class Checkbox : public Widget {
public:
    // Constructor
    explicit Checkbox(const std::string& id, const std::string& label = "", bool initialState = false);
    
    // Destructor
    ~Checkbox() override = default;

    // State management
    bool isChecked() const;
    void setChecked(bool checked);
    
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
    using CheckedChangedSignal = std::function<void(bool)>;
    void setOnCheckedChanged(CheckedChangedSignal callback);

private:
    std::string m_label;
    bool m_isChecked;
    bool m_isHovered;
    bool m_isFocused;
    CheckedChangedSignal m_onCheckedChanged;

    // Internal helpers
    void toggleState();
    void updateStyle();
};

} // namespace UI
} // namespace RebelCAD
