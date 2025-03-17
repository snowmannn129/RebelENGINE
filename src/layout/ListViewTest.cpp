#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include "ui/widgets/ListView.h"
#include "core/EventBus.h"

using namespace RebelCAD::UI;

class ListViewTest : public ::testing::Test {
protected:
    void SetUp() override {
        listView = std::make_shared<ListView>("test_listview");
    }

    void TearDown() override {
        listView.reset();
    }

    std::shared_ptr<ListView> listView;
};

// Construction tests
TEST_F(ListViewTest, DefaultConstructor) {
    EXPECT_EQ(listView->getId(), "test_listview");
    EXPECT_TRUE(listView->getItems().empty());
    EXPECT_EQ(listView->getSelectedIndices().size(), 0);
}

TEST_F(ListViewTest, ConstructorWithItems) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    ListView list("test_list", items);
    EXPECT_EQ(list.getItems(), items);
}

// Item management tests
TEST_F(ListViewTest, AddsItems) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    EXPECT_EQ(listView->getItems(), items);
}

TEST_F(ListViewTest, AddsItem) {
    listView->addItem("New Item");
    EXPECT_EQ(listView->getItems().size(), 1);
    EXPECT_EQ(listView->getItems()[0], "New Item");
}

TEST_F(ListViewTest, RemovesItem) {
    listView->addItem("Item 1");
    listView->addItem("Item 2");
    listView->removeItem(0);
    EXPECT_EQ(listView->getItems().size(), 1);
    EXPECT_EQ(listView->getItems()[0], "Item 2");
}

TEST_F(ListViewTest, ClearsItems) {
    listView->addItem("Item 1");
    listView->addItem("Item 2");
    listView->clearItems();
    EXPECT_TRUE(listView->getItems().empty());
}

// Selection tests
TEST_F(ListViewTest, SingleSelection) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    listView->setSelectionMode(ListView::SelectionMode::Single);
    
    listView->setSelectedIndex(1);
    auto selected = listView->getSelectedIndices();
    EXPECT_EQ(selected.size(), 1);
    EXPECT_EQ(*selected.begin(), 1);

    // Selecting another item should deselect the previous one
    listView->setSelectedIndex(2);
    selected = listView->getSelectedIndices();
    EXPECT_EQ(selected.size(), 1);
    EXPECT_EQ(*selected.begin(), 2);
}

TEST_F(ListViewTest, MultiSelection) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    listView->setSelectionMode(ListView::SelectionMode::Multiple);
    
    listView->addToSelection(0);
    listView->addToSelection(2);
    auto selected = listView->getSelectedIndices();
    EXPECT_EQ(selected.size(), 2);
    EXPECT_TRUE(selected.find(0) != selected.end());
    EXPECT_TRUE(selected.find(2) != selected.end());
}

// Event handling tests
TEST_F(ListViewTest, SelectionChangedEvent) {
    bool eventReceived = false;
    std::vector<size_t> eventIndices;
    
    listView->onSelectionChanged([&](const std::set<size_t>& indices) {
        eventReceived = true;
        eventIndices.assign(indices.begin(), indices.end());
    });

    listView->addItem("Item 1");
    listView->setSelectedIndex(0);
    
    EXPECT_TRUE(eventReceived);
    EXPECT_EQ(eventIndices.size(), 1);
    EXPECT_EQ(eventIndices[0], 0);
}

// Keyboard navigation tests
TEST_F(ListViewTest, KeyboardNavigation) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    
    // Test arrow down
    listView->handleKeyPress(ImGuiKey_DownArrow);
    EXPECT_EQ(*listView->getSelectedIndices().begin(), 0);
    
    listView->handleKeyPress(ImGuiKey_DownArrow);
    EXPECT_EQ(*listView->getSelectedIndices().begin(), 1);
    
    // Test arrow up
    listView->handleKeyPress(ImGuiKey_UpArrow);
    EXPECT_EQ(*listView->getSelectedIndices().begin(), 0);
}

// Style tests
TEST_F(ListViewTest, StyleApplication) {
    nlohmann::json style = {
        {"backgroundColor", "#FFFFFF"},
        {"textColor", "#000000"},
        {"selectedBackgroundColor", "#0000FF"},
        {"selectedTextColor", "#FFFFFF"},
        {"borderColor", "#CCCCCC"}
    };
    
    listView->applyStyle(style);
    EXPECT_EQ(listView->getStyle(), style);
}

// Layout tests
TEST_F(ListViewTest, PreferredSizeCalculation) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    
    ImVec2 size = listView->calculatePreferredSize();
    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
}

// Focus management tests
TEST_F(ListViewTest, FocusHandling) {
    EXPECT_TRUE(listView->isFocusable());
    
    listView->setFocus(true);
    EXPECT_TRUE(listView->hasFocus());
    
    listView->setFocus(false);
    EXPECT_FALSE(listView->hasFocus());
}

// Visibility tests
TEST_F(ListViewTest, VisibilityControl) {
    EXPECT_TRUE(listView->isVisible());
    
    listView->setVisible(false);
    EXPECT_FALSE(listView->isVisible());
    
    listView->setVisible(true);
    EXPECT_TRUE(listView->isVisible());
}

// Selection management tests
TEST_F(ListViewTest, RemoveFromSelection) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    listView->setSelectionMode(ListView::SelectionMode::Multiple);
    
    listView->addToSelection(0);
    listView->addToSelection(1);
    listView->removeFromSelection(0);
    
    auto selected = listView->getSelectedIndices();
    EXPECT_EQ(selected.size(), 1);
    EXPECT_TRUE(selected.find(1) != selected.end());
}

TEST_F(ListViewTest, ClearSelection) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    listView->setSelectionMode(ListView::SelectionMode::Multiple);
    
    listView->addToSelection(0);
    listView->addToSelection(1);
    listView->clearSelection();
    
    EXPECT_TRUE(listView->getSelectedIndices().empty());
}

TEST_F(ListViewTest, SelectionModeSwitch) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    listView->setSelectionMode(ListView::SelectionMode::Multiple);
    
    listView->addToSelection(0);
    listView->addToSelection(1);
    EXPECT_EQ(listView->getSelectedIndices().size(), 2);
    
    // Switching to single selection should keep only the first selected item
    listView->setSelectionMode(ListView::SelectionMode::Single);
    EXPECT_EQ(listView->getSelectedIndices().size(), 1);
    EXPECT_EQ(*listView->getSelectedIndices().begin(), 0);
}

// Extended keyboard navigation tests
TEST_F(ListViewTest, ExtendedKeyboardNavigation) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5"};
    listView->addItems(items);
    
    // Test Home key
    listView->setSelectedIndex(2);
    listView->handleKeyPress(ImGuiKey_Home);
    EXPECT_EQ(*listView->getSelectedIndices().begin(), 0);
    
    // Test End key
    listView->handleKeyPress(ImGuiKey_End);
    EXPECT_EQ(*listView->getSelectedIndices().begin(), 4);
}

// Index validation tests
TEST_F(ListViewTest, IndexValidation) {
    std::vector<std::string> items = {"Item 1", "Item 2"};
    listView->addItems(items);
    
    EXPECT_THROW(listView->setSelectedIndex(2), std::out_of_range);
    EXPECT_THROW(listView->addToSelection(3), std::out_of_range);
    EXPECT_THROW(listView->removeFromSelection(5), std::out_of_range);
    EXPECT_THROW(listView->removeItem(10), std::out_of_range);
}

// Scroll position tests
TEST_F(ListViewTest, ScrollPosition) {
    std::vector<std::string> items;
    for (int i = 0; i < 100; i++) {
        items.push_back("Item " + std::to_string(i));
    }
    listView->addItems(items);
    
    listView->setScrollPosition(50);
    EXPECT_EQ(listView->getScrollPosition(), 50);
    
    // Test scroll bounds
    listView->setScrollPosition(-10);
    EXPECT_EQ(listView->getScrollPosition(), 0);
    
    listView->setScrollPosition(1000);
    EXPECT_LT(listView->getScrollPosition(), 1000);
}

// New tests for ImGui integration
TEST_F(ListViewTest, ImGuiStyleIntegration) {
    nlohmann::json style = {
        {"backgroundColor", "#FF0000"},
        {"selectedBackgroundColor", "#00FF00"},
        {"selectedTextColor", "#0000FF"}
    };
    listView->applyStyle(style);
    
    // Verify style is applied correctly
    EXPECT_EQ(listView->getStyle()["backgroundColor"], "#FF0000");
    EXPECT_EQ(listView->getStyle()["selectedBackgroundColor"], "#00FF00");
    EXPECT_EQ(listView->getStyle()["selectedTextColor"], "#0000FF");
}

// Size and layout management tests
TEST_F(ListViewTest, CustomSizeHandling) {
    ImVec2 customSize(300.0f, 400.0f);
    listView->setSize(customSize);
    
    // Add enough items to exceed the custom height
    for (int i = 0; i < 30; i++) {
        listView->addItem("Item " + std::to_string(i));
    }
    
    // Verify scroll bounds are updated with custom size
    float maxScroll = listView->calculatePreferredSize().y - customSize.y;
    listView->setScrollPosition(maxScroll + 100);  // Try to scroll past the end
    EXPECT_EQ(listView->getScrollPosition(), maxScroll);
}

TEST_F(ListViewTest, PreferredSizeWithDifferentItemCounts) {
    // Test empty list
    ImVec2 emptySize = listView->calculatePreferredSize();
    EXPECT_GT(emptySize.x, 0);  // Should have minimum width
    EXPECT_GT(emptySize.y, 0);  // Should have minimum height
    
    // Test with few items
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    listView->addItems(items);
    ImVec2 fewItemsSize = listView->calculatePreferredSize();
    EXPECT_GT(fewItemsSize.y, emptySize.y);
    
    // Test with many items
    for (int i = 0; i < 100; i++) {
        listView->addItem("Item " + std::to_string(i));
    }
    ImVec2 manyItemsSize = listView->calculatePreferredSize();
    EXPECT_GT(manyItemsSize.y, fewItemsSize.y);
}
