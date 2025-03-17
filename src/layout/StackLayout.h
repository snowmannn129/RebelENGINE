#pragma once

#include "../Widget.h"
#include "imgui.h"
#include <vector>
#include <memory>

namespace RebelCAD {
namespace UI {

/**
 * @brief Stack Layout Manager
 * 
 * Arranges widgets in a vertical or horizontal stack.
 * Each widget takes up the full width/height in the perpendicular direction.
 */
class StackLayout {
public:
    /**
     * @brief Stack orientation options
     */
    enum class Orientation {
        Vertical,    ///< Stack widgets vertically
        Horizontal  ///< Stack widgets horizontally
    };

    /**
     * @brief Constructor
     * @param orientation The stacking direction
     */
    explicit StackLayout(Orientation orientation = Orientation::Vertical);

    /**
     * @brief Add a widget to the stack
     * @param widget Widget to add
     */
    void addWidget(std::shared_ptr<Widget> widget);

    /**
     * @brief Remove a widget from the stack
     * @param widget Widget to remove
     * @return true if widget was found and removed
     */
    bool removeWidget(std::shared_ptr<Widget> widget);

    /**
     * @brief Get all widgets in the stack
     * @return Vector of widget pointers
     */
    const std::vector<std::shared_ptr<Widget>>& widgets() const;

    /**
     * @brief Set spacing between widgets
     * @param spacing Pixel spacing between widgets
     */
    void setSpacing(float spacing);

    /**
     * @brief Get current spacing between widgets
     * @return Current spacing in pixels
     */
    float spacing() const;

    /**
     * @brief Set the orientation of the stack
     * @param orientation New orientation
     */
    void setOrientation(Orientation orientation);

    /**
     * @brief Get current orientation
     * @return Current orientation
     */
    Orientation orientation() const;

    /**
     * @brief Update layout of all widgets
     * Called when container size changes or widgets are added/removed
     * @param x Left position
     * @param y Top position
     * @param width Available width
     * @param height Available height
     */
    void updateLayout(float x, float y, float width, float height);

private:
    Orientation m_orientation;
    float m_spacing;
    std::vector<std::shared_ptr<Widget>> m_widgets;
};

} // namespace UI
} // namespace RebelCAD
