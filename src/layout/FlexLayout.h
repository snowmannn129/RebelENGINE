#pragma once

#include <vector>
#include <memory>
#include "../Widget.h"
#include "../ImGuiWrapper.h"

namespace RebelCAD {
namespace UI {

/**
 * @brief Represents a flexible box layout manager for dynamic widget arrangement
 * 
 * The FlexLayout class provides CSS Flexbox-like functionality for widget layout, with support for:
 * - Flexible direction (row/column)
 * - Wrapping behavior
 * - Content justification
 * - Item alignment
 * - Flexible growth and shrinking
 * - Dynamic space distribution
 */
class FlexLayout {
public:
    /**
     * @brief Direction of the flex container's main axis
     */
    enum class Direction {
        Row,            ///< Left to right in ltr
        RowReverse,     ///< Right to left in ltr
        Column,         ///< Top to bottom
        ColumnReverse   ///< Bottom to top
    };

    /**
     * @brief Wrapping behavior when items exceed container size
     */
    enum class Wrap {
        NoWrap,         ///< Single-line, may overflow
        Wrap,           ///< Multi-line, wrap as needed
        WrapReverse     ///< Multi-line, wrap opposite direction
    };

    /**
     * @brief Main axis alignment of items
     */
    enum class JustifyContent {
        Start,          ///< Pack items at start
        End,            ///< Pack items at end
        Center,         ///< Pack items at center
        SpaceBetween,   ///< Distribute space between items
        SpaceAround,    ///< Distribute space around items
        SpaceEvenly     ///< Equal space between and around items
    };

    /**
     * @brief Cross axis alignment of items
     */
    enum class AlignItems {
        Start,          ///< Pack items at start
        End,           ///< Pack items at end
        Center,         ///< Pack items at center
        Stretch,        ///< Stretch items to fill container
        Baseline        ///< Align by text baselines
    };

    /**
     * @brief Cross axis alignment of wrapped lines
     */
    enum class AlignContent {
        Start,          ///< Pack lines at start
        End,            ///< Pack lines at end
        Center,         ///< Pack lines at center
        Stretch,        ///< Stretch lines to fill container
        SpaceBetween,   ///< Distribute space between lines
        SpaceAround     ///< Distribute space around lines
    };

    /**
     * @brief Properties for individual flex items
     */
    struct FlexItemProperties {
        float grow{0};      ///< Growth factor
        float shrink{1};    ///< Shrink factor
        float basis{0};     ///< Base size
        AlignItems alignSelf{AlignItems::Start}; ///< Individual alignment
    };

    /**
     * @brief Constructs a FlexLayout with default properties
     */
    FlexLayout();
    ~FlexLayout() = default;

    /**
     * @brief Adds a widget to the flex container
     * @param widget Widget to add
     * @param properties Flex properties for this item
     * @return true if widget was added successfully
     */
    bool addWidget(std::shared_ptr<Widget> widget, 
                  const FlexItemProperties& properties = FlexItemProperties());

    /**
     * @brief Removes a widget from the flex container
     * @param widget Widget to remove
     * @return true if widget was removed successfully
     */
    bool removeWidget(std::shared_ptr<Widget> widget);

    /**
     * @brief Sets the flex direction
     * @param direction New direction
     */
    void setDirection(Direction direction);

    /**
     * @brief Sets the wrapping behavior
     * @param wrap New wrap mode
     */
    void setWrap(Wrap wrap);

    /**
     * @brief Sets the justify content mode
     * @param justify New justification mode
     */
    void setJustifyContent(JustifyContent justify);

    /**
     * @brief Sets the align items mode
     * @param align New alignment mode
     */
    void setAlignItems(AlignItems align);

    /**
     * @brief Sets the align content mode
     * @param align New content alignment mode
     */
    void setAlignContent(AlignContent align);

    /**
     * @brief Sets the gap between items
     * @param gap Gap size in pixels
     */
    void setGap(float gap);

    /**
     * @brief Updates the layout, recalculating positions and sizes
     * @param availableWidth Total available width
     * @param availableHeight Total available height
     */
    void update(float availableWidth, float availableHeight);

    /**
     * @brief Renders the flex container and its items using ImGui
     */
    void render();

private:
    struct FlexItem {
        std::shared_ptr<Widget> widget;
        FlexItemProperties properties;
        float computedMainSize;
        float computedCrossSize;
        float position[2];  // x, y coordinates
    };

    Direction m_direction;
    Wrap m_wrap;
    JustifyContent m_justifyContent;
    AlignItems m_alignItems;
    AlignContent m_alignContent;
    float m_gap;
    std::vector<FlexItem> m_items;
    std::vector<std::vector<FlexItem*>> m_lines;  // For wrap support

    /**
     * @brief Calculates flex layout for all items
     * @param availableWidth Total available width
     * @param availableHeight Total available height
     */
    void calculateLayout(float availableWidth, float availableHeight);

    /**
     * @brief Distributes free space among items according to flex rules
     * @param line Items in current line
     * @param freeSpace Available space to distribute
     */
    void distributeSpace(std::vector<FlexItem*>& line, float freeSpace);

    /**
     * @brief Aligns items on the cross axis
     * @param line Items in current line
     * @param lineHeight Height of the current line
     */
    void alignItemsInLine(std::vector<FlexItem*>& line, float lineHeight);

    /**
     * @brief Gets whether the main axis is horizontal
     * @return true if main axis is horizontal
     */
    bool isMainAxisHorizontal() const;
};

} // namespace UI
} // namespace RebelCAD
