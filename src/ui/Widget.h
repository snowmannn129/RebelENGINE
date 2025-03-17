#pragma once

#include <string>
#include <memory>
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <core/EventBus.h>

namespace RebelCAD {
namespace UI {

/**
 * Base class for all UI widgets in RebelCAD.
 * Provides common functionality for rendering, event handling, and state management.
 */
class Widget {
public:
    // Constructor and destructor
    explicit Widget(const std::string& id) : id_(id) {}
    virtual ~Widget() = default;

    // Core widget interface
    virtual void render() = 0;
    virtual ImVec2 calculatePreferredSize() const = 0;

    // Widget identification
    const std::string& getId() const { return id_; }

    // Style management
    virtual void applyStyle(const nlohmann::json& style) = 0;

    // Focus management
    virtual bool isFocusable() const { return false; }
    virtual void setFocus(bool focused) { hasFocus_ = focused; }
    virtual bool hasFocus() const { return hasFocus_; }

    // Visibility
    virtual bool isVisible() const { return isVisible_; }
    virtual void setVisible(bool visible) { isVisible_ = visible; }

    // Position and size
    void setPosition(const ImVec2& pos) { position_ = pos; }
    const ImVec2& getPosition() const { return position_; }
    void setSize(const ImVec2& size) { size_ = size; }
    const ImVec2& getSize() const { return size_; }

    // Enable/disable
    bool isEnabled() const { return isEnabled_; }
    void setEnabled(bool enabled) { isEnabled_ = enabled; }

protected:
    std::string id_;
    ImVec2 position_{0.0f, 0.0f};
    ImVec2 size_{0.0f, 0.0f};
    bool hasFocus_ = false;
    bool isVisible_ = true;
    bool isEnabled_ = true;

    // Event system access for derived classes
    Core::EventBus& getEventBus() { return eventBus_; }

private:
    Core::EventBus& eventBus_{Core::EventBus::getInstance()};

    // Prevent copying and assignment
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
};

} // namespace UI
} // namespace RebelCAD
