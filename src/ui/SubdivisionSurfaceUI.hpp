#pragma once

#include "modeling/SubdivisionSurface.hpp"
#include "graphics/Viewport.h"
#include "graphics/ViewportManager.h"
#include "graphics/SelectionManager.h"
#include "graphics/RenderableNode.h"
#include <memory>
#include <chrono>
#include <future>
#include <queue>
#include <mutex>

namespace RebelCAD {
namespace UI {

/**
 * @brief UI component for controlling subdivision surfaces
 * 
 * Provides interactive controls for:
 * - Subdivision level adjustment
 * - Sharp feature marking
 * - Real-time preview
 * - Cage visualization
 * - Performance statistics
 */
class SubdivisionSurfaceUI {
public:
    /**
     * @brief Constructs the subdivision surface UI
     * @param viewport The viewport to render in
     */
    SubdivisionSurfaceUI(
        std::shared_ptr<Graphics::Viewport> viewport,
        std::shared_ptr<Graphics::SelectionManager> selectionManager);

    /**
     * @brief Renders the UI controls and handles input
     * @param surface The subdivision surface to control
     */
    void render(std::shared_ptr<Modeling::SubdivisionSurface> surface);

    /**
     * @brief Sets whether to show the control cage
     */
    void setShowCage(bool show) { showCage_ = show; }

    /**
     * @brief Sets whether to show performance statistics
     */
    void setShowStats(bool show) { showStats_ = show; }

private:
    std::shared_ptr<Graphics::Viewport> viewport_;
    std::shared_ptr<Graphics::SelectionManager> selectionManager_;
    // UI State
    bool showCage_ = true;
    bool showStats_ = true;
    int subdivisionLevel_ = 0;
    bool previewEnabled_ = true;
    bool adaptiveSubdivisionEnabled_ = true;
    float adaptiveThreshold_ = 0.1f;
    bool isMarkingSharpFeatures_ = false;
    float currentSharpness_ = 1.0f;

    // Preview System
    struct PreviewCache {
        std::shared_ptr<Graphics::RenderableNode> node;
        int level;
        std::chrono::steady_clock::time_point lastUsed;
    };
    std::unordered_map<int, PreviewCache> previewCache_;
    std::mutex cacheMutex_;
    std::future<void> previewUpdateTask_;
    
    // Adaptive Subdivision
    struct AdaptiveRegion {
        glm::vec3 center;
        float radius;
        int targetLevel;
    };
    std::vector<AdaptiveRegion> adaptiveRegions_;

    // Performance tracking
    struct PerformanceStats {
        std::chrono::microseconds subdivisionTime{0};
        std::chrono::microseconds renderTime{0};
        size_t vertexCount = 0;
        size_t faceCount = 0;
        float memoryUsage = 0.0f; // In MB
        size_t cacheSize = 0;
        float adaptiveRegionCount = 0;
        float averageProcessingTime = 0.0f;
        float frameRate = 0.0f;
        std::vector<float> memoryHistory;
        std::vector<float> frameTimeHistory;
        std::vector<float> subdivisionTimeHistory;
        size_t historyMaxSize = 100;
        float subdivisionProgress = 0.0f;
        bool isSubdividing = false;
    } stats_;

    // Frame timing
    std::chrono::steady_clock::time_point lastFrameTime_;
    float frameTimeSmoothing_ = 0.1f; // For exponential moving average

    // Level of Detail
    struct LodTransition {
        int fromLevel;
        int toLevel;
        float progress;
        std::chrono::steady_clock::time_point startTime;
        float duration = 0.5f; // seconds
    };
    std::optional<LodTransition> currentTransition_;

    // Thread Pool
    static constexpr size_t MAX_THREADS = 4;
    std::vector<std::future<void>> workerTasks_;
    std::queue<std::function<void()>> taskQueue_;
    std::mutex taskMutex_;

    // Visual Feedback
    struct TemporaryNode {
        std::shared_ptr<Graphics::RenderableNode> node;
        std::chrono::steady_clock::time_point timestamp;
    };
    std::vector<TemporaryNode> temporaryNodes_;

    // UI Rendering
    void renderControlPanel(std::shared_ptr<Modeling::SubdivisionSurface> surface);
    void renderStatsPanel() const;
    void renderCage(std::shared_ptr<Modeling::SubdivisionSurface> surface) const;
    void renderSharpFeatureTools(std::shared_ptr<Modeling::SubdivisionSurface> surface);
    void renderAdaptiveControls();
    
    // Preview System
    void updatePreview(std::shared_ptr<Modeling::SubdivisionSurface> surface);
    void cleanupOldPreviews();
    std::shared_ptr<Graphics::RenderableNode> getCachedPreview(int level);
    void cachePreview(int level, std::shared_ptr<Graphics::RenderableNode> preview);
    std::shared_ptr<Graphics::RenderableNode> createPreviewNode(
        std::shared_ptr<Modeling::HalfEdgeMesh> mesh);
    
    // Adaptive Subdivision
    void updateAdaptiveRegions(std::shared_ptr<Modeling::SubdivisionSurface> surface);
    int calculateTargetLevel(const glm::vec3& position) const;
    void subdivideAdaptively(std::shared_ptr<Modeling::SubdivisionSurface> surface);
    
    // Performance & Threading
    void updateStats(std::shared_ptr<Modeling::SubdivisionSurface> surface);
    void handleSharpFeatureMarking(std::shared_ptr<Modeling::SubdivisionSurface> surface);
    void processTaskQueue();
    void scheduleTask(std::function<void()> task);
};

} // namespace UI
} // namespace RebelCAD
