#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "graphics/ViewportLayout.h"

using namespace RebelCAD::Graphics;

class ViewportLayoutTests : public ::testing::Test {
protected:
    void SetUp() override {
        layout = std::make_unique<ViewportLayout>(800, 600);
    }

    std::unique_ptr<ViewportLayout> layout;
};

// Test basic grid configuration
TEST_F(ViewportLayoutTests, GridConfiguration) {
    EXPECT_NO_THROW(layout->setGridSize(2, 2));
    EXPECT_THROW(layout->setGridSize(0, 2), std::invalid_argument);
    EXPECT_THROW(layout->setGridSize(2, 0), std::invalid_argument);
    EXPECT_THROW(layout->setGridSize(-1, -1), std::invalid_argument);
}

// Test viewport creation and cell validation
TEST_F(ViewportLayoutTests, ViewportCreation) {
    layout->setGridSize(2, 2);

    // Valid viewport creation
    ViewportCell cell1(0, 0, 1, 1);
    Viewport* vp1 = layout->addViewport(cell1);
    EXPECT_NE(vp1, nullptr);

    // Invalid cell position
    ViewportCell cell2(2, 2, 1, 1);
    EXPECT_THROW(layout->addViewport(cell2), std::invalid_argument);

    // Invalid cell span
    ViewportCell cell3(0, 0, 3, 3);
    EXPECT_THROW(layout->addViewport(cell3), std::invalid_argument);

    // Overlapping cells
    ViewportCell cell4(0, 0, 1, 1);
    EXPECT_THROW(layout->addViewport(cell4), std::invalid_argument);
}

// Test viewport removal
TEST_F(ViewportLayoutTests, ViewportRemoval) {
    layout->setGridSize(2, 2);

    ViewportCell cell(0, 0, 1, 1);
    Viewport* vp = layout->addViewport(cell);
    EXPECT_NE(vp, nullptr);

    layout->removeViewport(vp);
    EXPECT_EQ(layout->getViewport(0, 0), nullptr);

    // Test removing non-existent viewport
    EXPECT_NO_THROW(layout->removeViewport(nullptr));
}

// Test viewport lookup
TEST_F(ViewportLayoutTests, ViewportLookup) {
    layout->setGridSize(2, 2);

    ViewportCell cell1(0, 0, 1, 1);
    Viewport* vp1 = layout->addViewport(cell1);

    ViewportCell cell2(1, 1, 1, 1);
    Viewport* vp2 = layout->addViewport(cell2);

    EXPECT_EQ(layout->getViewport(0, 0), vp1);
    EXPECT_EQ(layout->getViewport(1, 1), vp2);
    EXPECT_EQ(layout->getViewport(0, 1), nullptr);
}

// Test layout presets
TEST_F(ViewportLayoutTests, LayoutPresets) {
    // Create a preset
    ViewportLayoutPreset preset;
    preset.name = "Test Preset";
    preset.rows = 2;
    preset.cols = 2;
    preset.cells = {
        ViewportCell(0, 0, 1, 1),
        ViewportCell(0, 1, 1, 1),
        ViewportCell(1, 0, 1, 2)
    };
    preset.projections = {
        ViewportProjection::Perspective,
        ViewportProjection::Orthographic,
        ViewportProjection::Perspective
    };
    preset.views = {
        ViewportView::Front,
        ViewportView::Top,
        ViewportView::Isometric
    };

    // Apply preset
    EXPECT_NO_THROW(layout->applyPreset(preset));

    // Verify layout
    EXPECT_NE(layout->getViewport(0, 0), nullptr);
    EXPECT_NE(layout->getViewport(0, 1), nullptr);
    EXPECT_NE(layout->getViewport(1, 0), nullptr);
}

// Test viewport synchronization
TEST_F(ViewportLayoutTests, ViewportSynchronization) {
    layout->setGridSize(2, 2);

    ViewportCell cell1(0, 0, 1, 1);
    Viewport* vp1 = layout->addViewport(cell1);

    ViewportCell cell2(0, 1, 1, 1);
    Viewport* vp2 = layout->addViewport(cell2);

    std::vector<Viewport*> viewports = {vp1, vp2};
    EXPECT_NO_THROW(layout->synchronizeViewports(viewports));

    // Verify synchronization
    EXPECT_TRUE(vp1->isSynchronizedWith(*vp2));
}

// Test thread safety
TEST_F(ViewportLayoutTests, ThreadSafety) {
    layout->setGridSize(3, 3);

    // Create viewports from multiple threads
    const int numThreads = 4;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i]() {
            ViewportCell cell(i % 3, i % 3, 1, 1);
            try {
                layout->addViewport(cell);
            } catch (const std::invalid_argument&) {
                // Expected for overlapping cells
            }
        });
    }

    // Resize window while creating viewports
    threads.emplace_back([this]() {
        for (int i = 0; i < 10; ++i) {
            layout->setDimensions(800 + i * 10, 600 + i * 10);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Update viewports while creating/resizing
    threads.emplace_back([this]() {
        for (int i = 0; i < 10; ++i) {
            layout->updateAll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify layout integrity
    EXPECT_NO_THROW(layout->updateAll());
}

// Test viewport dimensions
TEST_F(ViewportLayoutTests, ViewportDimensions) {
    layout->setGridSize(2, 2);

    ViewportCell cell(0, 0, 1, 1);
    Viewport* vp = layout->addViewport(cell);
    EXPECT_NE(vp, nullptr);

    // Check initial dimensions
    glm::vec4 rect = layout->getCellRect(cell);
    EXPECT_EQ(rect.z, 400); // 800/2
    EXPECT_EQ(rect.w, 300); // 600/2

    // Resize window
    layout->setDimensions(1000, 800);
    rect = layout->getCellRect(cell);
    EXPECT_EQ(rect.z, 500); // 1000/2
    EXPECT_EQ(rect.w, 400); // 800/2
}

// Test spanning cells
TEST_F(ViewportLayoutTests, SpanningCells) {
    layout->setGridSize(2, 2);

    ViewportCell cell(0, 0, 2, 2);
    Viewport* vp = layout->addViewport(cell);
    EXPECT_NE(vp, nullptr);

    // Check dimensions of spanning viewport
    glm::vec4 rect = layout->getCellRect(cell);
    EXPECT_EQ(rect.z, 800); // Full width
    EXPECT_EQ(rect.w, 600); // Full height

    // Try to add another viewport (should fail due to overlap)
    ViewportCell cell2(0, 0, 1, 1);
    EXPECT_THROW(layout->addViewport(cell2), std::invalid_argument);
}

// Test preset saving and loading
TEST_F(ViewportLayoutTests, PresetSaveLoad) {
    layout->setGridSize(2, 2);

    // Create some viewports
    layout->addViewport(ViewportCell(0, 0, 1, 1));
    layout->addViewport(ViewportCell(0, 1, 1, 1));
    layout->addViewport(ViewportCell(1, 0, 1, 2));

    // Save current layout as preset
    ViewportLayoutPreset saved = layout->saveAsPreset("Test");
    EXPECT_EQ(saved.name, "Test");
    EXPECT_EQ(saved.rows, 2);
    EXPECT_EQ(saved.cols, 2);
    EXPECT_EQ(saved.cells.size(), 3);

    // Clear layout
    layout = std::make_unique<ViewportLayout>(800, 600);

    // Apply saved preset
    EXPECT_NO_THROW(layout->applyPreset(saved));

    // Verify restored layout
    EXPECT_NE(layout->getViewport(0, 0), nullptr);
    EXPECT_NE(layout->getViewport(0, 1), nullptr);
    EXPECT_NE(layout->getViewport(1, 0), nullptr);
}
