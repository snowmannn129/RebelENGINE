#include "ui/ViewportUI.h"
#include "graphics/Viewport.h"
#include "imgui.h"
#include <glm/gtc/constants.hpp>
#include <string>

namespace RebelCAD {
namespace UI {

// ViewManipulationWidget Implementation
//------------------------------------

ViewManipulationWidget::ViewManipulationWidget(Graphics::ViewportManager* manager)
    : m_manager(manager)
    , m_visible(true)
    , m_isDragging(false)
    , m_orbitSpeed(0.01f)
    , m_panSpeed(0.005f)
    , m_zoomSpeed(0.1f)
{
}

void ViewManipulationWidget::render(Graphics::Viewport* viewport)
{
    if (!m_visible || !viewport) return;

    // Set up window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoSavedSettings |
                            ImGuiWindowFlags_AlwaysAutoResize;

    // Position the widget in the top-right corner of the viewport
    ImGui::SetNextWindowPos(
        ImVec2(viewport->getX() + viewport->getWidth() - 10,
               viewport->getY() + 10),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f)  // Top-right anchor
    );

    // Create the main widget window
    std::string windowId = "View Controls##" + std::to_string(reinterpret_cast<uintptr_t>(viewport));
    if (ImGui::Begin(windowId.c_str(), 
                     nullptr, flags))
    {
        // Standard view buttons in a grid
        renderViewButtons();
        
        ImGui::Separator();
        
        // Orbit controls
        renderOrbitControls();
        
        ImGui::Separator();
        
        // Projection toggle
        renderProjectionToggle();
    }
    ImGui::End();

    // Handle active manipulation if dragging
    if (m_isDragging)
    {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            handleOrbitDrag();
        }
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            handlePanDrag();
        }
        else
        {
            m_isDragging = false;
        }
    }

    // Handle zoom with mouse wheel
    handleZoom();
}

void ViewManipulationWidget::renderOrbitControls()
{
    ImGui::Text("Orbit Controls");
    
    // Orbit sphere visualization
    ImVec2 center = ImGui::GetCursorScreenPos();
    float radius = 30.0f;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    // Draw orbit sphere
    draw_list->AddCircle(
        ImVec2(center.x + radius, center.y + radius),
        radius,
        ImGui::GetColorU32(ImGuiCol_Text),
        32,
        2.0f
    );
    
    // Draw axes
    draw_list->AddLine(
        ImVec2(center.x + radius - 20, center.y + radius),
        ImVec2(center.x + radius + 20, center.y + radius),
        ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)),
        2.0f
    );
    draw_list->AddLine(
        ImVec2(center.x + radius, center.y + radius - 20),
        ImVec2(center.x + radius, center.y + radius + 20),
        ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)),
        2.0f
    );
    
    ImGui::Dummy(ImVec2(radius * 2 + 10, radius * 2 + 10));
    
    // Orbit speed control
    ImGui::SliderFloat("Orbit Speed", &m_orbitSpeed, 0.001f, 0.05f, "%.3f");
    ImGui::SliderFloat("Pan Speed", &m_panSpeed, 0.001f, 0.02f, "%.3f");
    ImGui::SliderFloat("Zoom Speed", &m_zoomSpeed, 0.01f, 0.5f, "%.3f");
}

void ViewManipulationWidget::renderViewButtons()
{
    ImGui::Text("Standard Views");
    
    // Create a 3x3 grid of view buttons
    float buttonWidth = 60.0f;
    float buttonHeight = 25.0f;
    
    auto createViewButton = [this](const char* label, Graphics::ViewportView view) {
        if (ImGui::Button(label, ImVec2(60.0f, 25.0f)))
        {
            Graphics::CameraPreset preset(label, view);
            m_manager->applyCameraPreset(nullptr, preset);  // Current viewport
        }
    };
    
    // Top row
    ImGui::BeginGroup();
    createViewButton("Back", Graphics::ViewportView::Back);
    ImGui::SameLine();
    createViewButton("Top", Graphics::ViewportView::Top);
    ImGui::SameLine();
    createViewButton("Front", Graphics::ViewportView::Front);
    ImGui::EndGroup();
    
    // Middle row
    ImGui::BeginGroup();
    createViewButton("Left", Graphics::ViewportView::Left);
    ImGui::SameLine();
    createViewButton("Iso", Graphics::ViewportView::Isometric);
    ImGui::SameLine();
    createViewButton("Right", Graphics::ViewportView::Right);
    ImGui::EndGroup();
    
    // Bottom row
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(buttonWidth, 0));  // Spacer
    ImGui::SameLine();
    createViewButton("Bottom", Graphics::ViewportView::Bottom);
    ImGui::EndGroup();
}

void ViewManipulationWidget::renderProjectionToggle()
{
    ImGui::Text("Projection");
    
    // Get current projection type from viewport
    auto viewport = m_manager->getViewport(0, 0); // Get current viewport
    if (!viewport) return;
    
    bool isPerspective = viewport->getProjectionType() == Graphics::ViewportProjection::Perspective;
    if (ImGui::RadioButton("Perspective", isPerspective))
    {
        // Create and apply a camera preset that maintains the current view but changes projection
        Graphics::CameraPreset preset("temp", Graphics::ViewportView::Custom,
                                    Graphics::ViewportProjection::Perspective);
        preset.fieldOfView = glm::radians(45.0f);
        m_manager->applyCameraPreset(viewport, preset);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Ortho", !isPerspective))
    {
        // Create and apply a camera preset that maintains the current view but changes projection
        Graphics::CameraPreset preset("temp", Graphics::ViewportView::Custom,
                                    Graphics::ViewportProjection::Orthographic);
        preset.orthoScale = 5.0f;
        m_manager->applyCameraPreset(viewport, preset);
    }
}

void ViewManipulationWidget::handleOrbitDrag()
{
    auto viewport = m_manager->getViewport(0, 0); // Get current viewport
    if (!viewport) return;

    ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
    {
        // Convert mouse movement to rotation angles
        float angleX = mouseDelta.y * m_orbitSpeed;
        float angleY = mouseDelta.x * m_orbitSpeed;
        
        // Create a view state for the rotation
        Graphics::ViewBookmark newView;
        newView.name = "orbit";
        
        // Get current camera state from viewport
        glm::vec3 currentPos = viewport->getEyePosition();
        glm::vec3 target = viewport->getTarget();
        
        // Rotate around Y axis first (angleY)
        glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), angleY, glm::vec3(0.0f, 1.0f, 0.0f));
        // Then rotate around local X axis (angleX)
        glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), angleX, glm::vec3(1.0f, 0.0f, 0.0f));
        
        glm::vec4 newPos = rotY * rotX * glm::vec4(currentPos - target, 1.0f);
        newView.cameraPosition = target + glm::vec3(newPos);
        newView.cameraTarget = target;
        newView.upVector = glm::vec3(0.0f, 1.0f, 0.0f);
        newView.projection = viewport->getProjectionType();
        
        // Apply the new view with a quick transition
        Graphics::ViewTransitionConfig transition;
        transition.duration = 0.1f; // Quick transition for smooth interaction
        m_manager->applyBookmark(viewport, newView, transition);
    }
}

void ViewManipulationWidget::handlePanDrag()
{
    auto viewport = m_manager->getViewport(0, 0); // Get current viewport
    if (!viewport) return;

    ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
    {
        // Convert mouse movement to pan distances
        float panX = mouseDelta.x * m_panSpeed;
        float panY = -mouseDelta.y * m_panSpeed;  // Invert Y for natural pan
        
        // Create a view state for the pan
        Graphics::ViewBookmark newView;
        newView.name = "pan";
        
        // Get current camera state from viewport
        glm::vec3 currentPos = viewport->getEyePosition();
        glm::vec3 currentTarget = viewport->getTarget();
        
        // Calculate right vector as cross product of up and view direction
        glm::vec3 viewDir = glm::normalize(currentTarget - currentPos);
        glm::vec3 right = glm::normalize(glm::cross(viewDir, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::cross(right, viewDir);
        
        // Scale pan speed based on projection type and distance
        float panScale = 1.0f;
        if (viewport->getProjectionType() == Graphics::ViewportProjection::Perspective) {
            panScale = glm::length(currentPos - currentTarget) * 0.001f;
        } else {
            panScale = 0.1f; // Fixed scale for orthographic
        }
        
        // Apply pan offsets
        glm::vec3 offset = (right * panX + up * panY) * panScale;
        newView.cameraPosition = currentPos + offset;
        newView.cameraTarget = currentTarget + offset;
        newView.upVector = glm::vec3(0.0f, 1.0f, 0.0f);
        newView.projection = viewport->getProjectionType();
        
        // Apply the new view with a quick transition
        Graphics::ViewTransitionConfig transition;
        transition.duration = 0.1f;
        m_manager->applyBookmark(viewport, newView, transition);
    }
}

void ViewManipulationWidget::handleZoom()
{
    auto viewport = m_manager->getViewport(0, 0); // Get current viewport
    if (!viewport) return;

    float mouseWheel = ImGui::GetIO().MouseWheel;
    if (mouseWheel != 0.0f)
    {
        float zoomFactor = 1.0f + (mouseWheel * m_zoomSpeed);
        
        // Create a view state for the zoom
        Graphics::ViewBookmark newView;
        newView.name = "zoom";
        
        // Get current camera state from viewport
        glm::vec3 currentPos = viewport->getEyePosition();
        glm::vec3 currentTarget = viewport->getTarget();
        
        if (viewport->getProjectionType() == Graphics::ViewportProjection::Perspective) {
            // For perspective, move camera position
            glm::vec3 viewDir = glm::normalize(currentPos - currentTarget);
            float distance = glm::length(currentPos - currentTarget);
            float newDistance = distance / zoomFactor;
            
            newView.cameraPosition = currentTarget + viewDir * newDistance;
            newView.cameraTarget = currentTarget;
        } else {
            // For orthographic, adjust scale
            newView.cameraPosition = currentPos;
            newView.cameraTarget = currentTarget;
            newView.orthoScale = viewport->getOrthoScale() / zoomFactor;
        }
        
        newView.upVector = glm::vec3(0.0f, 1.0f, 0.0f);
        newView.projection = viewport->getProjectionType();
        
        // Apply the new view with a quick transition
        Graphics::ViewTransitionConfig transition;
        transition.duration = 0.1f;
        m_manager->applyBookmark(viewport, newView, transition);
    }
}

void ViewManipulationWidget::setVisible(bool visible)
{
    m_visible = visible;
}

bool ViewManipulationWidget::isVisible() const
{
    return m_visible;
}

// LayoutConfigPanel Implementation
//-------------------------------

LayoutConfigPanel::LayoutConfigPanel(Graphics::ViewportManager* manager)
    : m_manager(manager)
    , m_visible(true)
    , m_gridRows(2)
    , m_gridCols(2)
    , m_newPresetName("")
{
}

void LayoutConfigPanel::render()
{
    if (!m_visible) return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Layout Configuration", &m_visible, flags))
    {
        // Preset selector section
        renderPresetSelector();
        
        ImGui::Separator();
        
        // Grid configuration section
        renderGridConfig();
        
        ImGui::Separator();
        
        // Save preset section
        renderSavePreset();
    }
    ImGui::End();
}

void LayoutConfigPanel::renderPresetSelector()
{
    ImGui::Text("Layout Presets");
    
    // Get list of presets from manager
    std::vector<std::string> presets = m_manager->getLayoutPresetNames();
    
    // Create combo box for preset selection
    static int currentPreset = 0;
    if (ImGui::BeginCombo("Select Preset", 
        currentPreset < presets.size() ? presets[currentPreset].c_str() : "Custom"))
    {
        for (int i = 0; i < presets.size(); i++)
        {
            bool isSelected = (currentPreset == i);
            if (ImGui::Selectable(presets[i].c_str(), isSelected))
            {
                currentPreset = i;
                // Apply the selected preset
                Graphics::ViewportLayoutPreset preset = m_manager->getLayoutPreset(presets[i]);
                m_manager->applyLayoutPreset(preset);
                
                // Update grid dimensions to match preset
                m_gridRows = preset.rows;
                m_gridCols = preset.cols;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void LayoutConfigPanel::renderGridConfig()
{
    ImGui::Text("Grid Configuration");
    
    // Grid dimension controls
    bool gridChanged = false;
    gridChanged |= ImGui::SliderInt("Rows", &m_gridRows, 1, 4);
    gridChanged |= ImGui::SliderInt("Columns", &m_gridCols, 1, 4);
    
    if (gridChanged)
    {
        // Preview grid layout
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float cellSize = 30.0f;
        float spacing = 2.0f;
        float totalWidth = m_gridCols * (cellSize + spacing);
        float totalHeight = m_gridRows * (cellSize + spacing);
        
        // Draw grid cells
        for (int row = 0; row < m_gridRows; row++)
        {
            for (int col = 0; col < m_gridCols; col++)
            {
                ImVec2 cellMin(
                    pos.x + col * (cellSize + spacing),
                    pos.y + row * (cellSize + spacing)
                );
                ImVec2 cellMax(
                    cellMin.x + cellSize,
                    cellMin.y + cellSize
                );
                draw_list->AddRect(cellMin, cellMax, IM_COL32(255, 255, 255, 100));
            }
        }
        
        ImGui::Dummy(ImVec2(totalWidth, totalHeight + 10));
    }
    
    if (ImGui::Button("Apply Grid"))
    {
        applyGridConfig();
    }
}

void LayoutConfigPanel::renderSavePreset()
{
    ImGui::Text("Save Layout Preset");
    
    // Input field for preset name
    char nameBuffer[128] = "";
    strncpy(nameBuffer, m_newPresetName.c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Preset Name", nameBuffer, sizeof(nameBuffer)))
    {
        m_newPresetName = nameBuffer;
    }
    
    if (ImGui::Button("Save Current Layout"))
    {
        if (!m_newPresetName.empty())
        {
            // Save current layout as preset
            m_manager->saveLayoutPreset(m_newPresetName);
            m_newPresetName.clear();
        }
    }
}

void LayoutConfigPanel::applyGridConfig()
{
    // Create a basic layout preset with the new grid dimensions
    Graphics::ViewportLayoutPreset preset("Custom", m_gridRows, m_gridCols);
    
    // Add default cells
    for (int row = 0; row < m_gridRows; row++)
    {
        for (int col = 0; col < m_gridCols; col++)
        {
            preset.cells.emplace_back(row, col);
            preset.projections.push_back(Graphics::ViewportProjection::Perspective);
            preset.views.push_back(Graphics::ViewportView::Isometric);
        }
    }
    
    // Apply the new layout
    m_manager->applyLayoutPreset(preset);
}

void LayoutConfigPanel::setVisible(bool visible)
{
    m_visible = visible;
}

bool LayoutConfigPanel::isVisible() const
{
    return m_visible;
}

// CameraPresetSelector Implementation
//----------------------------------

CameraPresetSelector::CameraPresetSelector(Graphics::ViewportManager* manager)
    : m_manager(manager)
    , m_visible(true)
    , m_newPresetName("")
    , m_transitionConfig()
{
}

void CameraPresetSelector::render(Graphics::Viewport* viewport)
{
    if (!m_visible) return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Camera Presets", &m_visible, flags))
    {
        // Preset list section
        renderPresetList();
        
        ImGui::Separator();
        
        // Preset creation section
        renderPresetCreation();
        
        ImGui::Separator();
        
        // Transition configuration section
        renderTransitionConfig();
    }
    ImGui::End();
}

void CameraPresetSelector::renderPresetList()
{
    ImGui::Text("Camera Presets");
    
    // Get list of presets from manager
    std::vector<std::string> presets = m_manager->getPresetNames();
    
    // Create list box for preset selection
    static int currentPreset = -1;
    ImGui::BeginChild("PresetList", ImVec2(200, 150), true);
    for (int i = 0; i < presets.size(); i++)
    {
        if (ImGui::Selectable(presets[i].c_str(), currentPreset == i))
        {
            currentPreset = i;
            // Apply the selected preset with current transition settings
            if (currentPreset >= 0 && currentPreset < presets.size())
            {
                Graphics::CameraPreset preset = m_manager->getCameraPreset(presets[currentPreset]);
                m_manager->applyCameraPreset(nullptr, preset, m_transitionConfig);
            }
        }
    }
    ImGui::EndChild();
}

void CameraPresetSelector::renderPresetCreation()
{
    ImGui::Text("Create New Preset");
    
    // Input field for preset name
    char nameBuffer[128] = "";
    strncpy(nameBuffer, m_newPresetName.c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Preset Name", nameBuffer, sizeof(nameBuffer)))
    {
        m_newPresetName = nameBuffer;
    }
    
    if (ImGui::Button("Save Current View"))
    {
        if (!m_newPresetName.empty())
        {
            // Save current view as preset
            m_manager->saveCameraPreset(nullptr, m_newPresetName);
            m_newPresetName.clear();
        }
    }
}

void CameraPresetSelector::renderTransitionConfig()
{
    ImGui::Text("Transition Settings");
    
    // Duration slider
    ImGui::SliderFloat("Duration (s)", &m_transitionConfig.duration, 0.1f, 2.0f, "%.1f");
    
    // Smoothing toggles
    ImGui::Checkbox("Smooth Rotation", &m_transitionConfig.smoothRotation);
    ImGui::Checkbox("Smooth Zoom", &m_transitionConfig.smoothZoom);
    
    // Easing factors
    ImGui::SliderFloat("Ease In", &m_transitionConfig.easeInFactor, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Ease Out", &m_transitionConfig.easeOutFactor, 0.0f, 1.0f, "%.2f");
}

void CameraPresetSelector::setVisible(bool visible)
{
    m_visible = visible;
}

bool CameraPresetSelector::isVisible() const
{
    return m_visible;
}

// ViewBookmarkManager Implementation
//--------------------------------

ViewBookmarkManager::ViewBookmarkManager(Graphics::ViewportManager* manager)
    : m_manager(manager)
    , m_visible(true)
    , m_newBookmarkName("")
    , m_transitionConfig()
{
}

void ViewBookmarkManager::render(Graphics::Viewport* viewport)
{
    if (!m_visible) return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("View Bookmarks", &m_visible, flags))
    {
        // Bookmark list section
        renderBookmarkList();
        
        ImGui::Separator();
        
        // Bookmark creation section
        renderBookmarkCreation();
        
        ImGui::Separator();
        
        // Transition configuration section
        renderTransitionConfig();
    }
    ImGui::End();
}

void ViewBookmarkManager::renderBookmarkList()
{
    ImGui::Text("Saved Bookmarks");
    
    // Get list of bookmarks from manager
    std::vector<std::string> bookmarks = m_manager->getBookmarkNames();
    
    // Create list box for bookmark selection
    static int currentBookmark = -1;
    ImGui::BeginChild("BookmarkList", ImVec2(200, 150), true);
    for (int i = 0; i < bookmarks.size(); i++)
    {
        if (ImGui::Selectable(bookmarks[i].c_str(), currentBookmark == i))
        {
            currentBookmark = i;
            // Apply the selected bookmark with current transition settings
            if (currentBookmark >= 0 && currentBookmark < bookmarks.size())
            {
                Graphics::ViewBookmark bookmark = m_manager->getBookmark(bookmarks[currentBookmark]);
                m_manager->applyBookmark(nullptr, bookmark, m_transitionConfig);
            }
        }
        
        // Add delete button
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) // Right click
        {
            ImGui::OpenPopup(("BookmarkContext##" + std::to_string(i)).c_str());
        }
        
        if (ImGui::BeginPopup(("BookmarkContext##" + std::to_string(i)).c_str()))
        {
            if (ImGui::MenuItem("Delete"))
            {
                m_manager->deleteBookmark(bookmarks[i]);
                if (currentBookmark == i)
                {
                    currentBookmark = -1;
                }
            }
            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();
}

void ViewBookmarkManager::renderBookmarkCreation()
{
    ImGui::Text("Create New Bookmark");
    
    // Input field for bookmark name
    char nameBuffer[128] = "";
    strncpy(nameBuffer, m_newBookmarkName.c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Bookmark Name", nameBuffer, sizeof(nameBuffer)))
    {
        m_newBookmarkName = nameBuffer;
    }
    
    if (ImGui::Button("Save Current View"))
    {
        if (!m_newBookmarkName.empty())
        {
            // Save current view as bookmark
            m_manager->createBookmark(nullptr, m_newBookmarkName);
            m_newBookmarkName.clear();
        }
    }
}

void ViewBookmarkManager::renderTransitionConfig()
{
    ImGui::Text("Transition Settings");
    
    // Duration slider
    ImGui::SliderFloat("Duration (s)", &m_transitionConfig.duration, 0.1f, 2.0f, "%.1f");
    
    // Smoothing toggles
    ImGui::Checkbox("Smooth Rotation", &m_transitionConfig.smoothRotation);
    ImGui::Checkbox("Smooth Zoom", &m_transitionConfig.smoothZoom);
    
    // Easing factors
    ImGui::SliderFloat("Ease In", &m_transitionConfig.easeInFactor, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Ease Out", &m_transitionConfig.easeOutFactor, 0.0f, 1.0f, "%.2f");
}

void ViewBookmarkManager::setVisible(bool visible)
{
    m_visible = visible;
}

bool ViewBookmarkManager::isVisible() const
{
    return m_visible;
}

// ViewFeedbackSystem Implementation
//--------------------------------

ViewFeedbackSystem::ViewFeedbackSystem(Graphics::ViewportManager* manager)
    : m_manager(manager)
{
}

void ViewFeedbackSystem::update()
{
    updateFeedbackTimers();
    
    // Render active feedback for each viewport
    for (const auto& [viewport, state] : m_feedback)
    {
        if (state.active)
        {
            renderFeedback(viewport, state);
        }
    }
}

void ViewFeedbackSystem::showTransitionFeedback(Graphics::Viewport* viewport, float progress)
{
    if (!viewport) return;
    
    FeedbackState state;
    state.active = true;
    state.duration = 0.5f;
    state.elapsed = 0.0f;
    state.message = "Transitioning... " + std::to_string(int(progress * 100)) + "%";
    
    m_feedback[viewport] = state;
}

void ViewFeedbackSystem::showViewChangeFeedback(Graphics::Viewport* viewport, Graphics::ViewportView view)
{
    if (!viewport) return;
    
    FeedbackState state;
    state.active = true;
    state.duration = 1.0f;
    state.elapsed = 0.0f;
    
    // Convert view enum to string
    std::string viewName;
    switch (view)
    {
        case Graphics::ViewportView::Front: viewName = "Front"; break;
        case Graphics::ViewportView::Back: viewName = "Back"; break;
        case Graphics::ViewportView::Top: viewName = "Top"; break;
        case Graphics::ViewportView::Bottom: viewName = "Bottom"; break;
        case Graphics::ViewportView::Left: viewName = "Left"; break;
        case Graphics::ViewportView::Right: viewName = "Right"; break;
        case Graphics::ViewportView::Isometric: viewName = "Isometric"; break;
        default: viewName = "Custom"; break;
    }
    
    state.message = "View: " + viewName;
    m_feedback[viewport] = state;
}

void ViewFeedbackSystem::showProjectionChangeFeedback(Graphics::Viewport* viewport, Graphics::ViewportProjection proj)
{
    if (!viewport) return;
    
    FeedbackState state;
    state.active = true;
    state.duration = 1.0f;
    state.elapsed = 0.0f;
    state.message = "Projection: " + std::string(proj == Graphics::ViewportProjection::Perspective ? "Perspective" : "Orthographic");
    
    m_feedback[viewport] = state;
}

void ViewFeedbackSystem::renderFeedback(Graphics::Viewport* viewport, const FeedbackState& state)
{
    // Calculate fade based on elapsed time
    float alpha = 1.0f;
    if (state.elapsed > state.duration * 0.7f)  // Start fading out at 70% of duration
    {
        alpha = 1.0f - ((state.elapsed - (state.duration * 0.7f)) / (state.duration * 0.3f));
    }
    
    // Set up window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoInputs |
                            ImGuiWindowFlags_AlwaysAutoResize;
    
    // Position feedback at the top-center of the viewport
    ImGui::SetNextWindowPos(
        ImVec2(viewport->getX() + viewport->getWidth() * 0.5f,
               viewport->getY() + 10),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.0f)  // Center-top anchor
    );
    
    // Set window background alpha
    ImGui::SetNextWindowBgAlpha(0.7f * alpha);
    
    // Create unique window name using viewport pointer
    std::string windowId = "Feedback##" + std::to_string(reinterpret_cast<uintptr_t>(viewport));
    
    if (ImGui::Begin(windowId.c_str(), nullptr, flags))
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, alpha), "%s", state.message.c_str());
    }
    ImGui::End();
}

void ViewFeedbackSystem::updateFeedbackTimers()
{
    static auto lastUpdate = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
    lastUpdate = now;
    
    // Update and remove expired feedback
    for (auto it = m_feedback.begin(); it != m_feedback.end();)
    {
        auto& state = it->second;
        state.elapsed += deltaTime;
        
        if (state.elapsed >= state.duration)
        {
            it = m_feedback.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace UI
} // namespace RebelCAD
