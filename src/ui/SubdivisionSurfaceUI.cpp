#include "ui/SubdivisionSurfaceUI.hpp"
#include "graphics/RenderableNode.h"
#include "graphics/SelectionManager.h"
#include "graphics/ViewportManager.h"
#include "modeling/HalfEdgeMesh.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <thread>
#include <future>
#include <numeric>

namespace ImGui {
    void HelpMarker(const char* desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

namespace RebelCAD {
namespace UI {

SubdivisionSurfaceUI::SubdivisionSurfaceUI(
    std::shared_ptr<Graphics::Viewport> viewport,
    std::shared_ptr<Graphics::SelectionManager> selectionManager)
    : viewport_(viewport)
    , selectionManager_(selectionManager) {
    if (!viewport) {
        throw Error::InvalidArgument("Viewport cannot be null");
    }

    // Add selection filters
    selectionManager_->addFilter("vertices", [](const Graphics::SceneNode::Ptr& node) {
        return dynamic_cast<Modeling::HalfEdgeMesh::Vertex*>(node.get()) != nullptr;
    });
    selectionManager_->addFilter("edges", [](const Graphics::SceneNode::Ptr& node) {
        return dynamic_cast<Modeling::HalfEdgeMesh::Edge*>(node.get()) != nullptr;
    });
    selectionManager_->addFilter("faces", [](const Graphics::SceneNode::Ptr& node) {
        return dynamic_cast<Modeling::HalfEdgeMesh::Face*>(node.get()) != nullptr;
    });
    selectionManager_->addFilter("sharp_features", [](const Graphics::SceneNode::Ptr& node) {
        if (auto edge = dynamic_cast<Modeling::HalfEdgeMesh::Edge*>(node.get())) {
            return edge->isSharp;
        }
        if (auto vertex = dynamic_cast<Modeling::HalfEdgeMesh::Vertex*>(node.get())) {
            return vertex->isSharp;
        }
        return false;
    });

    // Create common selection groups
    selectionManager_->createGroup("sharp_edges", {});
    selectionManager_->createGroup("sharp_vertices", {});
    selectionManager_->createGroup("boundary_edges", {});
    selectionManager_->createGroup("boundary_vertices", {});
}

void SubdivisionSurfaceUI::render(std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    if (!surface) return;

    // Process any pending tasks
    processTaskQueue();

    // Begin dockable window
    ImGui::Begin("Subdivision Surface Controls");

    // Selection filters
    if (ImGui::CollapsingHeader("Selection Filters")) {
        bool verticesEnabled = true;
        bool edgesEnabled = true;
        bool facesEnabled = true;
        bool sharpFeaturesEnabled = true;

        if (ImGui::Checkbox("Vertices", &verticesEnabled)) {
            selectionManager_->enableFilter("vertices", verticesEnabled);
        }
        if (ImGui::Checkbox("Edges", &edgesEnabled)) {
            selectionManager_->enableFilter("edges", edgesEnabled);
        }
        if (ImGui::Checkbox("Faces", &facesEnabled)) {
            selectionManager_->enableFilter("faces", facesEnabled);
        }
        if (ImGui::Checkbox("Sharp Features", &sharpFeaturesEnabled)) {
            selectionManager_->enableFilter("sharp_features", sharpFeaturesEnabled);
        }
    }

    // Selection history and persistence
    if (ImGui::CollapsingHeader("Selection History")) {
        if (ImGui::Button("Undo Selection")) {
            selectionManager_->undo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Redo Selection")) {
            selectionManager_->redo();
        }

        if (ImGui::Button("Save Selection")) {
            ImGui::OpenPopup("Save Selection State");
        }
        ImGui::SameLine();
        if (ImGui::Button("Load Selection")) {
            ImGui::OpenPopup("Load Selection State");
        }

        // Save selection popup
        if (ImGui::BeginPopupModal("Save Selection State", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static char filename[256] = "selection.json";
            ImGui::InputText("Filename", filename, sizeof(filename));
            
            if (ImGui::Button("Save", ImVec2(120, 0))) {
                selectionManager_->saveSelectionState(filename);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Load selection popup
        if (ImGui::BeginPopupModal("Load Selection State", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static char filename[256] = "selection.json";
            ImGui::InputText("Filename", filename, sizeof(filename));
            
            if (ImGui::Button("Load", ImVec2(120, 0))) {
                selectionManager_->loadSelectionState(filename);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // Selection groups
    if (ImGui::CollapsingHeader("Selection Groups")) {
        if (ImGui::Button("Select Sharp Edges")) {
            auto mesh = surface->getBaseMesh();
            std::vector<Graphics::SceneNode::Ptr> sharpEdges;
            for (const auto& edge : mesh->getEdges()) {
                if (edge->isSharp) {
                    sharpEdges.push_back(edge);
                }
            }
            selectionManager_->select(sharpEdges);
            selectionManager_->addToGroup("sharp_edges", sharpEdges);
        }

        if (ImGui::Button("Select Sharp Vertices")) {
            auto mesh = surface->getBaseMesh();
            std::vector<Graphics::SceneNode::Ptr> sharpVertices;
            for (const auto& vertex : mesh->getVertices()) {
                if (vertex->isSharp) {
                    sharpVertices.push_back(vertex);
                }
            }
            selectionManager_->select(sharpVertices);
            selectionManager_->addToGroup("sharp_vertices", sharpVertices);
        }

        // Display existing groups
        const auto& groups = selectionManager_->getGroups();
        for (const auto& [name, group] : groups) {
            if (ImGui::TreeNode(name.c_str())) {
                ImGui::Text("Items: %zu", group.nodes.size());
                
                bool isVisible = group.isVisible;
                if (ImGui::Checkbox("Visible", &isVisible)) {
                    selectionManager_->setGroupVisibility(name, isVisible);
                }

                bool isLocked = group.isLocked;
                if (ImGui::Checkbox("Locked", &isLocked)) {
                    selectionManager_->setGroupLocked(name, isLocked);
                }

                if (ImGui::Button("Select Group")) {
                    selectionManager_->selectGroup(name);
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Group")) {
                    selectionManager_->deleteGroup(name);
                }

                ImGui::TreePop();
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Select Boundary")) {
            auto mesh = surface->getBaseMesh();
            std::vector<Graphics::SceneNode::Ptr> boundaryEdges;
            std::vector<Graphics::SceneNode::Ptr> boundaryVertices;
            
            for (const auto& edge : mesh->getEdges()) {
                if (!edge->halfEdge->pair) {
                    boundaryEdges.push_back(edge);
                    boundaryVertices.push_back(edge->halfEdge->vertex);
                    boundaryVertices.push_back(edge->halfEdge->prev->vertex);
                }
            }
            
            selectionManager_->select(boundaryEdges);
            selectionManager_->addToGroup("boundary_edges", boundaryEdges);
            selectionManager_->addToGroup("boundary_vertices", boundaryVertices);
        }
    }

    // Main controls
    renderControlPanel(surface);

    // Adaptive subdivision controls
    if (ImGui::CollapsingHeader("Adaptive Subdivision")) {
        renderAdaptiveControls();
    }

    // Sharp feature tools
    if (ImGui::CollapsingHeader("Sharp Features")) {
        renderSharpFeatureTools(surface);
    }

    // Performance stats
    if (showStats_) {
        ImGui::Separator();
        renderStatsPanel();
    }

    ImGui::End();

    // Update preview if enabled
    if (previewEnabled_) {
        updatePreview(surface);
    }

    // Update adaptive regions
    if (adaptiveSubdivisionEnabled_) {
        updateAdaptiveRegions(surface);
        subdivideAdaptively(surface);
    }

    // Cleanup old previews
    cleanupOldPreviews();

    // Update performance stats
    updateStats(surface);

    // Render control cage if enabled
    if (showCage_) {
        renderCage(surface);
    }

    // Handle sharp feature marking if in marking mode
    if (isMarkingSharpFeatures_) {
        handleSharpFeatureMarking(surface);
    }
}

void SubdivisionSurfaceUI::renderControlPanel(std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    // Subdivision level control with memory warning
    int level = surface->getSubdivisionLevel();
    float expectedMemory = 0.0f;
    if (ImGui::SliderInt("Subdivision Level", &level, 0, 5)) {
        // Calculate expected memory usage
        size_t currentFaces = surface->getBaseMesh()->getFaces().size();
        size_t expectedFaces = currentFaces * static_cast<size_t>(std::pow(4, level - surface->getSubdivisionLevel()));
        expectedMemory = (expectedFaces * sizeof(void*) * 4 + // Face pointers
                         expectedFaces * 4 * sizeof(glm::vec3)) // Vertices and normals
                         / (1024.0f * 1024.0f); // Convert to MB

        if (expectedMemory > 1000.0f) { // Warning if over 1GB
            ImGui::OpenPopup("Memory Warning");
        } else {
            if (level > surface->getSubdivisionLevel()) {
                surface->subdivide(level - surface->getSubdivisionLevel());
            }
        }
    }
    
    // Memory warning popup
    if (ImGui::BeginPopupModal("Memory Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Warning: This operation may require significant memory (%.1f MB)", expectedMemory);
        ImGui::Text("Consider using adaptive subdivision for better performance.");
        ImGui::Separator();
        
        if (ImGui::Button("Proceed", ImVec2(120, 0))) {
            if (level > surface->getSubdivisionLevel()) {
                surface->subdivide(level - surface->getSubdivisionLevel());
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Topology warnings
    auto mesh = surface->getBaseMesh();
    bool hasNonManifold = false;
    bool hasTJunctions = false;
    
    // Check for non-manifold edges
    for (const auto& edge : mesh->getEdges()) {
        auto he = edge->halfEdge;
        if (!he->pair || he->pair->pair != he) {
            hasNonManifold = true;
            break;
        }
    }
    
    // Check for T-junctions
    for (const auto& vertex : mesh->getVertices()) {
        auto he = vertex->outgoingHalfEdge;
        auto start = he;
        int edgeCount = 0;
        do {
            edgeCount++;
            he = he->next->pair;
        } while (he && he != start);
        
        if (he != start) { // Incomplete loop
            hasTJunctions = true;
            break;
        }
    }
    
    if (hasNonManifold || hasTJunctions) {
        ImGui::Spacing();
        if (hasNonManifold) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Warning: Non-manifold geometry detected");
            ImGui::SameLine();
            ImGui::HelpMarker("Non-manifold geometry may cause artifacts in subdivision");
        }
        if (hasTJunctions) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Warning: T-junctions detected");
            ImGui::SameLine();
            ImGui::HelpMarker("T-junctions may cause gaps in the subdivided surface");
            
            if (ImGui::Button("Fix T-junctions")) {
                surface->fixTJunctions();
            }
        }
    }

    // Preview toggle
    ImGui::Checkbox("Enable Preview", &previewEnabled_);

    // Control cage toggle
    ImGui::Checkbox("Show Control Cage", &showCage_);

    // Performance stats toggle
    ImGui::Checkbox("Show Performance Stats", &showStats_);

    // Sharp feature marking tools
    if (ImGui::CollapsingHeader("Sharp Features")) {
        static float sharpness = 1.0f;
        ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, 1.0f);

        if (ImGui::Button("Mark Sharp Edge")) {
            // Select edges for marking
            std::vector<Graphics::SceneNode::Ptr> nodes;
            selectionManager_->select(nodes, Graphics::SelectionMode::Single);
        }

        if (ImGui::Button("Mark Sharp Vertex")) {
            // Select vertices for marking
            std::vector<Graphics::SceneNode::Ptr> nodes;
            selectionManager_->select(nodes, Graphics::SelectionMode::Single);
        }

        if (ImGui::Button("Clear Sharp Features")) {
            auto mesh = surface->getBaseMesh();
            for (const auto& edge : mesh->getEdges()) {
                edge->isSharp = false;
                edge->sharpness = 0.0f;
            }
            for (const auto& vertex : mesh->getVertices()) {
                vertex->isSharp = false;
            }
        }
    }
}

void SubdivisionSurfaceUI::renderStatsPanel() const {
    if (ImGui::CollapsingHeader("Performance Statistics")) {
        // Basic stats
        ImGui::Text("Frame Rate: %.1f FPS", stats_.frameRate);
        ImGui::Text("Subdivision Time: %.3f ms", 
            stats_.subdivisionTime.count() / 1000.0f);
        ImGui::Text("Render Time: %.3f ms", 
            stats_.renderTime.count() / 1000.0f);
        ImGui::Text("Vertex Count: %zu", stats_.vertexCount);
        ImGui::Text("Face Count: %zu", stats_.faceCount);
        ImGui::Text("Memory Usage: %.2f MB", stats_.memoryUsage);

        // Subdivision progress
        if (stats_.isSubdividing) {
            ImGui::ProgressBar(stats_.subdivisionProgress, ImVec2(-1, 0));
            ImGui::Text("Subdividing... %.1f%%", stats_.subdivisionProgress * 100.0f);
        }

        // History graphs
        if (ImGui::TreeNode("Performance History")) {
            // Memory usage history
            if (!stats_.memoryHistory.empty()) {
                ImGui::PlotLines("Memory Usage (MB)", 
                    stats_.memoryHistory.data(),
                    static_cast<int>(stats_.memoryHistory.size()),
                    0, nullptr, 0.0f, FLT_MAX, ImVec2(-1, 80));
            }

            // Frame time history
            if (!stats_.frameTimeHistory.empty()) {
                ImGui::PlotLines("Frame Time (ms)",
                    stats_.frameTimeHistory.data(),
                    static_cast<int>(stats_.frameTimeHistory.size()),
                    0, nullptr, 0.0f, 33.3f, ImVec2(-1, 80));
            }

            // Subdivision time history
            if (!stats_.subdivisionTimeHistory.empty()) {
                ImGui::PlotLines("Subdivision Time (ms)",
                    stats_.subdivisionTimeHistory.data(),
                    static_cast<int>(stats_.subdivisionTimeHistory.size()),
                    0, nullptr, 0.0f, FLT_MAX, ImVec2(-1, 80));
            }

            ImGui::TreePop();
        }
    }
}

void SubdivisionSurfaceUI::renderCage(
    std::shared_ptr<Modeling::SubdivisionSurface> surface) const {
    auto mesh = surface->getBaseMesh();
    if (!mesh) return;

    // Convert mesh data to renderable format
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    // Extract vertices and create triangles
    for (const auto& face : mesh->getFaces()) {
        auto start = face->halfEdge;
        auto current = start;
        size_t baseIndex = vertices.size();
        
        // Collect face vertices
        do {
            vertices.push_back(current->vertex->position);
            normals.push_back(glm::vec3(0.0f)); // Will compute later
            current = current->next;
        } while (current != start);

        // Create triangles (assuming quads)
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
    }

    // Compute normals
    for (size_t i = 0; i < indices.size(); i += 3) {
        auto& v1 = vertices[indices[i]];
        auto& v2 = vertices[indices[i + 1]];
        auto& v3 = vertices[indices[i + 2]];
        auto normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
        normals[indices[i]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;
    }

    // Normalize all normals
    for (auto& normal : normals) {
        normal = glm::normalize(normal);
    }

    // Create and setup renderable node
    auto node = Graphics::RenderableNode::create("CageNode");
    node->setVertices(vertices);
    node->setNormals(normals);
    node->setIndices(indices);
    node->setColor(glm::vec4(0.2f, 0.6f, 1.0f, 0.5f));

    // Enable wireframe mode using OpenGL state
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    node->render();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SubdivisionSurfaceUI::updateStats(
    std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    auto mesh = surface->getBaseMesh();
    if (!mesh) return;

    // Update frame timing
    auto currentTime = std::chrono::steady_clock::now();
    if (lastFrameTime_ != std::chrono::steady_clock::time_point()) {
        float frameTime = std::chrono::duration<float>(currentTime - lastFrameTime_).count();
        float smoothedFrameTime = frameTime * frameTimeSmoothing_ + 
            (1.0f - frameTimeSmoothing_) * (1.0f / stats_.frameRate);
        stats_.frameRate = 1.0f / smoothedFrameTime;

        // Update frame time history
        stats_.frameTimeHistory.push_back(frameTime * 1000.0f); // Convert to ms
        if (stats_.frameTimeHistory.size() > stats_.historyMaxSize) {
            stats_.frameTimeHistory.erase(stats_.frameTimeHistory.begin());
        }
    }
    lastFrameTime_ = currentTime;

    // Update basic stats
    stats_.vertexCount = mesh->getVertices().size();
    stats_.faceCount = mesh->getFaces().size();

    // Estimate memory usage (rough approximation)
    size_t vertexMemory = stats_.vertexCount * sizeof(glm::vec3);
    size_t faceMemory = stats_.faceCount * sizeof(void*) * 4; // Assuming quads
    size_t totalMemory = vertexMemory + faceMemory;
    stats_.memoryUsage = static_cast<float>(totalMemory) / (1024.0f * 1024.0f);

    // Update memory history
    stats_.memoryHistory.push_back(stats_.memoryUsage);
    if (stats_.memoryHistory.size() > stats_.historyMaxSize) {
        stats_.memoryHistory.erase(stats_.memoryHistory.begin());
    }

    // Update subdivision time history
    if (stats_.subdivisionTime.count() > 0) {
        stats_.subdivisionTimeHistory.push_back(
            static_cast<float>(stats_.subdivisionTime.count()) / 1000.0f); // Convert to ms
        if (stats_.subdivisionTimeHistory.size() > stats_.historyMaxSize) {
            stats_.subdivisionTimeHistory.erase(stats_.subdivisionTimeHistory.begin());
        }
    }
}

void SubdivisionSurfaceUI::renderSharpFeatureTools(std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    ImGui::Checkbox("Mark Sharp Features", &isMarkingSharpFeatures_);
    
    ImGui::SliderFloat("Sharpness", &currentSharpness_, 0.0f, 1.0f);
    ImGui::SameLine();
    ImGui::HelpMarker("Controls the sharpness of marked edges and vertices");

    if (ImGui::Button("Mark Sharp Edge")) {
        isMarkingSharpFeatures_ = true;
        selectionManager_->setSelectionMode(Graphics::SelectionMode::Edge);
    }
    ImGui::SameLine();
    if (ImGui::Button("Mark Sharp Vertex")) {
        isMarkingSharpFeatures_ = true;
        selectionManager_->setSelectionMode(Graphics::SelectionMode::Vertex);
    }

    if (ImGui::Button("Clear All Sharp Features")) {
        auto mesh = surface->getBaseMesh();
        for (const auto& edge : mesh->getEdges()) {
            edge->isSharp = false;
            edge->sharpness = 0.0f;
        }
        for (const auto& vertex : mesh->getVertices()) {
            vertex->isSharp = false;
        }
        isMarkingSharpFeatures_ = false;
    }

    // Display current sharp feature stats
    auto mesh = surface->getBaseMesh();
    size_t sharpEdges = std::count_if(mesh->getEdges().begin(), mesh->getEdges().end(),
        [](const auto& edge) { return edge->isSharp; });
    size_t sharpVertices = std::count_if(mesh->getVertices().begin(), mesh->getVertices().end(),
        [](const auto& vertex) { return vertex->isSharp; });
    
    ImGui::Text("Sharp Edges: %zu", sharpEdges);
    ImGui::Text("Sharp Vertices: %zu", sharpVertices);
}

void SubdivisionSurfaceUI::renderAdaptiveControls() {
    ImGui::Checkbox("Enable Adaptive Subdivision", &adaptiveSubdivisionEnabled_);
    
    if (adaptiveSubdivisionEnabled_) {
        ImGui::SliderFloat("Detail Threshold", &adaptiveThreshold_, 0.01f, 0.5f);
        ImGui::SameLine();
        ImGui::HelpMarker("Lower values create more detailed subdivisions in high-curvature areas");
        
        ImGui::Text("Adaptive Regions: %zu", adaptiveRegions_.size());
        ImGui::Text("Average Subdivision Level: %.1f", 
            adaptiveRegions_.empty() ? 0.0f : 
            std::accumulate(adaptiveRegions_.begin(), adaptiveRegions_.end(), 0.0f,
                [](float sum, const auto& region) { return sum + region.targetLevel; }) / 
                adaptiveRegions_.size());
    }
}

std::shared_ptr<Graphics::RenderableNode> SubdivisionSurfaceUI::createPreviewNode(
    std::shared_ptr<Modeling::HalfEdgeMesh> mesh) {
    if (!mesh) return nullptr;

    // Convert mesh data to renderable format
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    // Extract vertices and create triangles
    for (const auto& face : mesh->getFaces()) {
        auto start = face->halfEdge;
        auto current = start;
        size_t baseIndex = vertices.size();
        
        // Collect face vertices
        do {
            vertices.push_back(current->vertex->position);
            normals.push_back(glm::vec3(0.0f)); // Will compute later
            current = current->next;
        } while (current != start);

        // Create triangles (assuming quads)
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
    }

    // Compute normals
    for (size_t i = 0; i < indices.size(); i += 3) {
        auto& v1 = vertices[indices[i]];
        auto& v2 = vertices[indices[i + 1]];
        auto& v3 = vertices[indices[i + 2]];
        auto normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
        normals[indices[i]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;
    }

    // Normalize all normals
    for (auto& normal : normals) {
        normal = glm::normalize(normal);
    }

    // Create and setup renderable node
    auto node = Graphics::RenderableNode::create("PreviewNode");
    node->setVertices(vertices);
    node->setNormals(normals);
    node->setIndices(indices);
    node->setColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    return node;
}

void SubdivisionSurfaceUI::updatePreview(std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    if (!previewEnabled_ || !surface) return;

    // Handle level of detail transition
    if (currentTransition_) {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - currentTransition_->startTime).count();
        currentTransition_->progress = std::min(elapsed / currentTransition_->duration, 1.0f);

        // Get previews for both levels
        auto fromPreview = getCachedPreview(currentTransition_->fromLevel);
        auto toPreview = getCachedPreview(currentTransition_->toLevel);

        if (fromPreview && toPreview) {
            // Blend between levels using progress
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            fromPreview->setColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f - currentTransition_->progress));
            fromPreview->render();
            
            toPreview->setColor(glm::vec4(0.8f, 0.8f, 0.8f, currentTransition_->progress));
            toPreview->render();
            
            glDisable(GL_BLEND);
        }

        // End transition when complete
        if (currentTransition_->progress >= 1.0f) {
            currentTransition_ = std::nullopt;
        }
        return;
    }

    // Check if we need to update the preview
    auto currentPreview = getCachedPreview(subdivisionLevel_);
    if (currentPreview) {
        // Use cached preview
        currentPreview->render();
        return;
    }

    // Schedule preview update in background
    if (!previewUpdateTask_.valid() || 
        previewUpdateTask_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        stats_.isSubdividing = true;
        stats_.subdivisionProgress = 0.0f;
        
        previewUpdateTask_ = std::async(std::launch::async, [this, surface]() {
            auto subdivided = surface->subdivide(subdivisionLevel_);
            
            // Update progress as subdivision proceeds
            for (float progress = 0.0f; progress < 1.0f; progress += 0.1f) {
                stats_.subdivisionProgress = progress;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            
            auto preview = createPreviewNode(subdivided);
            cachePreview(subdivisionLevel_, preview);
            
            stats_.subdivisionProgress = 1.0f;
            stats_.isSubdividing = false;
        });
    }
}

void SubdivisionSurfaceUI::cleanupOldPreviews() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto now = std::chrono::steady_clock::now();
    
    // Remove previews older than 5 seconds
    auto it = previewCache_.begin();
    while (it != previewCache_.end()) {
        if (now - it->second.lastUsed > std::chrono::seconds(5)) {
            it = previewCache_.erase(it);
        } else {
            ++it;
        }
    }
}

std::shared_ptr<Graphics::RenderableNode> SubdivisionSurfaceUI::getCachedPreview(int level) {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto it = previewCache_.find(level);
    if (it != previewCache_.end()) {
        it->second.lastUsed = std::chrono::steady_clock::now();
        return it->second.node;
    }
    return nullptr;
}

void SubdivisionSurfaceUI::cachePreview(int level, std::shared_ptr<Graphics::RenderableNode> preview) {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    PreviewCache cache{
        preview,
        level,
        std::chrono::steady_clock::now()
    };
    previewCache_[level] = std::move(cache);
}

void SubdivisionSurfaceUI::updateAdaptiveRegions(
    std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    if (!adaptiveSubdivisionEnabled_) return;

    // Clear old regions
    adaptiveRegions_.clear();

    // Get camera parameters
    glm::vec3 cameraPos = viewport_->getEyePosition();
    glm::vec3 viewDir = viewport_->getViewDirection();
    float viewDistance = glm::length(viewDir);

    // Get view frustum parameters
    float aspect = viewport_->getAspectRatio();
    float nearPlane = viewport_->getNearPlane();
    float farPlane = viewport_->getFarPlane();

    // Early exit if no surface
    if (!surface) return;

    // Create regions based on surface analysis
    auto mesh = surface->getBaseMesh();
    if (!mesh) return;

    for (const auto& face : mesh->getFaces()) {
        // Calculate face center and normal
        glm::vec3 faceCenter(0.0f);
        glm::vec3 faceNormal(0.0f);
        int vertCount = 0;
        auto start = face->halfEdge;
        auto current = start;
        
        std::vector<glm::vec3> faceVertices;
        do {
            faceVertices.push_back(current->vertex->position);
            faceCenter += current->vertex->position;
            vertCount++;
            current = current->next;
        } while (current != start);
        
        faceCenter /= static_cast<float>(vertCount);
        
        // Calculate face normal
        if (vertCount >= 3) {
            faceNormal = glm::normalize(glm::cross(
                faceVertices[1] - faceVertices[0],
                faceVertices[2] - faceVertices[0]
            ));
        }

        // Calculate view-dependent factors
        float distanceToCamera = glm::distance(faceCenter, cameraPos);
        float viewAngle = glm::dot(glm::normalize(faceCenter - cameraPos), faceNormal);
        
        // Skip faces facing away from camera or outside view frustum
        if (viewAngle <= 0.0f || distanceToCamera > farPlane) continue;

        // Calculate curvature
        float maxCurvature = 0.0f;
        current = start;
        do {
            // Compare normals of adjacent faces
            auto adjacentEdge = current->pair;
            if (adjacentEdge) {
                auto adjacentFace = adjacentEdge->face;
                if (adjacentFace) {
                    // Calculate adjacent face normal
                    glm::vec3 adjNormal(0.0f);
                    auto adjStart = adjacentFace->halfEdge;
                    auto adjCurrent = adjStart;
                    std::vector<glm::vec3> adjVertices;
                    do {
                        adjVertices.push_back(adjCurrent->vertex->position);
                        adjCurrent = adjCurrent->next;
                    } while (adjCurrent != adjStart);

                    if (adjVertices.size() >= 3) {
                        adjNormal = glm::normalize(glm::cross(
                            adjVertices[1] - adjVertices[0],
                            adjVertices[2] - adjVertices[0]
                        ));
                    }

                    // Curvature approximation using normal difference
                    float curvature = 1.0f - glm::dot(faceNormal, adjNormal);
                    maxCurvature = glm::max(maxCurvature, curvature);
                }
            }
            current = current->next;
        } while (current != start);

        // Create adaptive region if curvature is significant
        if (maxCurvature > adaptiveThreshold_) {
            float radius = glm::mix(2.0f, 5.0f, maxCurvature);
            int targetLevel = glm::clamp(
                static_cast<int>(maxCurvature * 5.0f / adaptiveThreshold_),
                1, 5
            );

            // Adjust based on view distance
            float distanceFactor = 1.0f - glm::clamp(distanceToCamera / farPlane, 0.0f, 1.0f);
            targetLevel = glm::max(1, static_cast<int>(targetLevel * distanceFactor));

            AdaptiveRegion region{
                faceCenter,
                radius,
                targetLevel
            };
            adaptiveRegions_.push_back(region);
        }
    }

    // Merge overlapping regions
    for (size_t i = 0; i < adaptiveRegions_.size(); ++i) {
        for (size_t j = i + 1; j < adaptiveRegions_.size(); /* no increment */) {
            auto& regionA = adaptiveRegions_[i];
            auto& regionB = adaptiveRegions_[j];

            float distance = glm::distance(regionA.center, regionB.center);
            if (distance < (regionA.radius + regionB.radius) * 0.5f) {
                // Merge regions
                regionA.center = (regionA.center + regionB.center) * 0.5f;
                regionA.radius = glm::max(regionA.radius, regionB.radius);
                regionA.targetLevel = glm::max(regionA.targetLevel, regionB.targetLevel);
                
                // Remove merged region
                adaptiveRegions_.erase(adaptiveRegions_.begin() + j);
            } else {
                ++j;
            }
        }
    }
}

int SubdivisionSurfaceUI::calculateTargetLevel(const glm::vec3& position) const {
    // Get camera position and view direction
    glm::vec3 cameraPos = viewport_->getEyePosition();
    
    // Calculate distance from camera to position
    float distance = glm::distance(cameraPos, position);
    
    // Base level calculation on distance
    // Closer objects get higher subdivision levels
    float normalizedDist = glm::clamp(distance / 100.0f, 0.0f, 1.0f);
    
    // Adjust based on adaptive threshold
    // Lower threshold means more detail
    float detailFactor = 1.0f - (adaptiveThreshold_ / 0.5f);
    
    // Calculate final level (0-5)
    // Further distances and higher thresholds reduce subdivision level
    int level = static_cast<int>(5.0f * (1.0f - normalizedDist) * detailFactor);
    
    // Clamp to valid range
    return glm::clamp(level, 0, 5);
}

void SubdivisionSurfaceUI::subdivideAdaptively(std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    if (!adaptiveSubdivisionEnabled_ || adaptiveRegions_.empty()) return;

    // Schedule adaptive subdivision in background
    scheduleTask([this, surface]() {
        auto mesh = surface->getBaseMesh();
        for (const auto& region : adaptiveRegions_) {
            // Find faces within region
            for (const auto& face : mesh->getFaces()) {
                // Calculate face center
                glm::vec3 center(0.0f);
                int vertCount = 0;
                auto start = face->halfEdge;
                auto current = start;
                do {
                    center += current->vertex->position;
                    vertCount++;
                    current = current->next;
                } while (current != start);
                center /= static_cast<float>(vertCount);

                // Check if face is within region
                float dist = glm::distance(center, region.center);
                if (dist <= region.radius) {
                    // Calculate subdivision level based on distance
                    float t = dist / region.radius;
                    int level = static_cast<int>((1.0f - t) * region.targetLevel);
                    surface->subdivide(level);
                }
            }
        }
    });
}

void SubdivisionSurfaceUI::processTaskQueue() {
    std::lock_guard<std::mutex> lock(taskMutex_);
    
    // Remove completed tasks
    workerTasks_.erase(
        std::remove_if(workerTasks_.begin(), workerTasks_.end(),
            [](auto& task) {
                return task.valid() && 
                       task.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
            }),
        workerTasks_.end());

    // Start new tasks if we have capacity
    while (!taskQueue_.empty() && workerTasks_.size() < MAX_THREADS) {
        auto task = std::move(taskQueue_.front());
        taskQueue_.pop();
        workerTasks_.push_back(std::async(std::launch::async, std::move(task)));
    }
}

void SubdivisionSurfaceUI::scheduleTask(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(taskMutex_);
    taskQueue_.push(std::move(task));
}

void SubdivisionSurfaceUI::handleSharpFeatureMarking(
    std::shared_ptr<Modeling::SubdivisionSurface> surface) {
    if (!isMarkingSharpFeatures_) return;

    // Get selected nodes
    const auto& selectedNodes = selectionManager_->getSelectedNodes();
    
    // Process selected nodes
    for (const auto& node : selectedNodes) {
        // Check if node represents an edge or vertex
        if (auto edge = std::dynamic_pointer_cast<Modeling::HalfEdgeMesh::Edge>(node)) {
            edge->isSharp = true;
            edge->sharpness = currentSharpness_;
            
            // Visual feedback - highlight the marked edge
            auto renderNode = Graphics::RenderableNode::create("SharpEdge");
            std::vector<glm::vec3> vertices = {
                edge->halfEdge->vertex->position,
                edge->halfEdge->next->vertex->position
            };
            std::vector<unsigned int> indices = {0, 1};
            std::vector<glm::vec3> normals(2, glm::vec3(0.0f, 1.0f, 0.0f)); // Default up vector
            renderNode->setVertices(vertices);
            renderNode->setNormals(normals);
            renderNode->setIndices(indices);
            renderNode->setColor(glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
            
            // Store the node for temporary display
            temporaryNodes_.push_back({
                renderNode,
                std::chrono::steady_clock::now()
            });
            
        } else if (auto vertex = std::dynamic_pointer_cast<Modeling::HalfEdgeMesh::Vertex>(node)) {
            vertex->isSharp = true;
            
            // Visual feedback - highlight the marked vertex
            auto renderNode = Graphics::RenderableNode::create("SharpVertex");
            std::vector<glm::vec3> vertices = { vertex->position };
            std::vector<unsigned int> indices = {0};
            std::vector<glm::vec3> normals(1, glm::vec3(0.0f, 1.0f, 0.0f)); // Default up vector
            renderNode->setVertices(vertices);
            renderNode->setNormals(normals);
            renderNode->setIndices(indices);
            renderNode->setColor(glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
            
            // Store the node for temporary display
            temporaryNodes_.push_back({
                renderNode,
                std::chrono::steady_clock::now()
            });
        }
        
        // Update stats immediately
        stats_.isSubdividing = true;
        stats_.subdivisionProgress = 0.0f;
        
        // Schedule a preview update
        scheduleTask([this, surface]() {
            updatePreview(surface);
            stats_.isSubdividing = false;
            stats_.subdivisionProgress = 1.0f;
        });
    }
    
    // Clear selection after processing
    if (!selectedNodes.empty()) {
        selectionManager_->clearSelection();
    }
    
    // Clean up old temporary nodes
    auto now = std::chrono::steady_clock::now();
    temporaryNodes_.erase(
        std::remove_if(temporaryNodes_.begin(), temporaryNodes_.end(),
            [now](const auto& temp) {
                return now - temp.timestamp > std::chrono::seconds(2);
            }),
        temporaryNodes_.end());
    
    // Render temporary nodes with modern OpenGL
    glEnable(GL_PROGRAM_POINT_SIZE); // Enable shader-controlled point size
    glEnable(GL_LINE_SMOOTH);        // Enable line antialiasing
    
    for (const auto& temp : temporaryNodes_) {
        if (auto node = temp.node) {
            // Calculate fade based on time
            float age = std::chrono::duration<float>(now - temp.timestamp).count();
            float alpha = 1.0f - (age / 2.0f); // Fade over 2 seconds
            glm::vec4 color = node->getColor();
            color.a = alpha;
            node->setColor(color);
            
            // Render with appropriate primitive type
            if (node->getName() == "SharpEdge") {
                glLineWidth(3.0f);
                node->render();
                glLineWidth(1.0f);
            } else if (node->getName() == "SharpVertex") {
                // Point size is controlled by shader
                node->render();
            }
        }
    }
    
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_LINE_SMOOTH);
}

} // namespace UI
} // namespace RebelCAD
