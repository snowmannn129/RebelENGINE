#include "graphics/SelectionInteractionManager.h"
#include "graphics/ObjectPicker.h"
#include "graphics/Viewport.h"
#include "graphics/SceneNode.h"
#include <algorithm>
#include <unordered_set>

namespace RebelCAD {
namespace Graphics {

SelectionInteractionManager::SelectionInteractionManager(
    std::shared_ptr<ObjectPicker> picker,
    std::shared_ptr<Viewport> viewport
) : m_picker(picker)
  , m_viewport(viewport)
  , m_currentMode(SelectionMode::Point)
  , m_currentModifier(SelectionModifier::None)
  , m_config()
  , m_history(std::make_unique<SelectionHistoryManager>())
  , m_filterManager(std::make_unique<SelectionFilterManager>())
  , m_groupManager(std::make_unique<SelectionGroupManager>())
  , m_persistenceManager(std::make_unique<SelectionPersistenceManager>())
{
    m_smartSelectionState.similarityThreshold = m_config.smartSelectionTolerance;
    m_smartSelectionState.autoComplete = m_config.autoCompleteSmartSelection;

    // Set up history callback
    m_history->setSelectionChangeCallback(
        [this](const std::vector<SceneNode::Ptr>& selection) {
            m_currentSelection = selection;
            if (m_selectionChangeCallback) {
                m_selectionChangeCallback(selection);
            }
        }
    );

    // Initialize standard filters
    m_filterManager->createStandardFilters();
}

bool SelectionInteractionManager::saveSelectionState(
    const std::string& name,
    const std::string& description
) {
    if (m_currentSelection.empty()) {
        return false;
    }

    bool result = m_persistenceManager->saveState(
        name,
        m_currentSelection,
        description
    );

    if (result) {
        recordSelectionOperation(
            m_currentSelection,
            "Save Selection State '" + name + "'"
        );
    }

    return result;
}

bool SelectionInteractionManager::loadSelectionState(const std::string& name) {
    auto selection = m_persistenceManager->loadState(name);
    if (selection.empty()) {
        return false;
    }

    // Apply filters to loaded selection
    auto filtered = applyFilters(selection);

    // Record operation and update selection
    recordSelectionOperation(
        filtered,
        "Load Selection State '" + name + "'"
    );

    return true;
}

bool SelectionInteractionManager::saveSelectionsToDisk(const std::string& filepath) {
    return m_persistenceManager->saveStatesToDisk(filepath);
}

bool SelectionInteractionManager::loadSelectionsFromDisk(const std::string& filepath) {
    return m_persistenceManager->loadStatesFromDisk(filepath);
}

bool SelectionInteractionManager::createSelectionGroup(const std::string& name) {
    if (m_currentSelection.empty()) {
        return false;
    }

    bool result = m_groupManager->createGroup(
        name,
        m_currentSelection,
        "Selection group created from " + std::to_string(m_currentSelection.size()) + " objects"
    );

    if (result) {
        recordSelectionOperation(
            m_currentSelection,
            "Create Selection Group '" + name + "'"
        );
    }

    return result;
}

bool SelectionInteractionManager::addSelectionToGroup(const std::string& name) {
    if (m_currentSelection.empty()) {
        return false;
    }

    bool result = m_groupManager->addToGroup(name, m_currentSelection);

    if (result) {
        recordSelectionOperation(
            m_currentSelection,
            "Add to Group '" + name + "'"
        );
    }

    return result;
}

bool SelectionInteractionManager::removeSelectionFromGroup(const std::string& name) {
    if (m_currentSelection.empty()) {
        return false;
    }

    bool result = m_groupManager->removeFromGroup(name, m_currentSelection);

    if (result) {
        recordSelectionOperation(
            m_currentSelection,
            "Remove from Group '" + name + "'"
        );
    }

    return result;
}

bool SelectionInteractionManager::selectGroup(const std::string& name) {
    auto members = m_groupManager->getGroupMembers(name);
    if (members.empty()) {
        return false;
    }

    // Apply filters to group members
    auto filtered = applyFilters(members);

    // Record operation and update selection
    recordSelectionOperation(
        filtered,
        "Select Group '" + name + "'"
    );

    return true;
}

void SelectionInteractionManager::setMode(SelectionMode mode) {
    if (m_currentMode != mode) {
        cancelSelection();
        m_currentMode = mode;
    }
}

void SelectionInteractionManager::setModifier(SelectionModifier modifier) {
    m_currentModifier = modifier;
    if (m_selectionState.active) {
        updatePreview();
    }
}

void SelectionInteractionManager::startSelection(float x, float y) {
    if (m_selectionState.active) {
        cancelSelection();
    }

    m_selectionState.active = true;
    m_selectionState.startX = x;
    m_selectionState.startY = y;
    m_selectionState.currentX = x;
    m_selectionState.currentY = y;
    m_selectionState.points.clear();
    m_selectionState.points.push_back(glm::vec2(x, y));

    if (m_currentMode == SelectionMode::Point) {
        auto selected = handlePointSelection(x, y);
        if (!selected.empty()) {
            m_selectionState.preview = applyModifier(selected);
            updatePreview();
        }
    }

    updateFeedback();
}

void SelectionInteractionManager::updateSelection(float x, float y) {
    if (!m_selectionState.active) return;

    m_selectionState.currentX = x;
    m_selectionState.currentY = y;

    switch (m_currentMode) {
        case SelectionMode::Box:
            {
                auto selected = handleBoxSelection(
                    m_selectionState.startX,
                    m_selectionState.startY,
                    x, y
                );
                m_selectionState.preview = applyModifier(selected);
            }
            break;

        case SelectionMode::Lasso:
            m_selectionState.points.push_back(glm::vec2(x, y));
            {
                auto selected = handleLassoSelection(m_selectionState.points);
                m_selectionState.preview = applyModifier(selected);
            }
            break;

        case SelectionMode::Paint:
            m_selectionState.points.push_back(glm::vec2(x, y));
            {
                auto selected = handlePaintSelection(m_selectionState.points);
                m_selectionState.preview = applyModifier(selected);
            }
            break;

        case SelectionMode::Smart:
            {
                auto selected = handleSmartSelection(x, y);
                m_selectionState.preview = applyModifier(selected);
            }
            break;

        default:
            break;
    }

    updatePreview();
    updateFeedback();
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::completeSelection(
    float x,
    float y
) {
    if (!m_selectionState.active) return {};

    updateSelection(x, y);
    std::vector<SceneNode::Ptr> result = m_selectionState.preview;
    
    // Record operation in history
    std::string description;
    switch (m_currentMode) {
        case SelectionMode::Point:
            description = "Point Selection";
            break;
        case SelectionMode::Box:
            description = "Box Selection";
            break;
        case SelectionMode::Lasso:
            description = "Lasso Selection";
            break;
        case SelectionMode::Paint:
            description = "Paint Selection";
            break;
        case SelectionMode::Smart:
            description = "Smart Selection";
            break;
    }

    switch (m_currentModifier) {
        case SelectionModifier::Add:
            description += " (Add)";
            break;
        case SelectionModifier::Subtract:
            description += " (Subtract)";
            break;
        case SelectionModifier::Intersect:
            description += " (Intersect)";
            break;
        default:
            break;
    }

    recordSelectionOperation(result, description);
    clearSelectionState();
    return result;
}

void SelectionInteractionManager::recordSelectionOperation(
    const std::vector<SceneNode::Ptr>& newSelection,
    const std::string& description
) {
    m_history->recordOperation(m_currentSelection, newSelection, description);
    m_currentSelection = newSelection;
    
    if (m_selectionChangeCallback) {
        m_selectionChangeCallback(newSelection);
    }
}

void SelectionInteractionManager::cancelSelection() {
    if (!m_selectionState.active) return;

    clearSelectionState();
    updatePreview();
    updateFeedback();
}

void SelectionInteractionManager::setConfig(const SelectionConfig& config) {
    m_config = config;
    m_smartSelectionState.similarityThreshold = config.smartSelectionTolerance;
    m_smartSelectionState.autoComplete = config.autoCompleteSmartSelection;
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::handlePointSelection(
    float x,
    float y
) {
    m_picker->setPickingPrecision(m_config.pointRadius);
    return m_picker->pickAtPoint(
        glm::vec2(x, y),
        m_viewport->getViewMatrix(),
        m_viewport->getProjectionMatrix()
    );
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::handleBoxSelection(
    float startX,
    float startY,
    float endX,
    float endY
) {
    return m_picker->pickInBox(
        glm::vec2(std::min(startX, endX), std::min(startY, endY)),
        glm::vec2(std::max(startX, endX), std::max(startY, endY)),
        m_viewport->getViewMatrix(),
        m_viewport->getProjectionMatrix()
    );
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::handleLassoSelection(
    const std::vector<glm::vec2>& points
) {
    return m_picker->pickInLasso(
        points,
        m_viewport->getViewMatrix(),
        m_viewport->getProjectionMatrix()
    );
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::handlePaintSelection(
    const std::vector<glm::vec2>& brushPoints
) {
    std::unordered_set<SceneNode::Ptr> selected;
    
    m_picker->setPickingPrecision(m_config.paintBrushSize);
    for (const auto& point : brushPoints) {
        auto result = m_picker->pickAtPoint(
            point,
            m_viewport->getViewMatrix(),
            m_viewport->getProjectionMatrix()
        );
        selected.insert(result.begin(), result.end());
    }

    return std::vector<SceneNode::Ptr>(selected.begin(), selected.end());
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::handleSmartSelection(
    float x,
    float y
) {
    // Get initial selection
    m_picker->setPickingPrecision(m_config.pointRadius);
    auto initialSelection = m_picker->pickAtPoint(
        glm::vec2(x, y),
        m_viewport->getViewMatrix(),
        m_viewport->getProjectionMatrix()
    );

    if (initialSelection.empty()) return {};

    // Use first selected object as seed
    if (m_smartSelectionState.seedObjects.empty()) {
        m_smartSelectionState.seedObjects = initialSelection;
    }

    // Find similar objects based on properties
    std::vector<SceneNode::Ptr> similarObjects;
    
    // Get all pickable objects in viewport
    auto allObjects = m_picker->pickInBox(
        glm::vec2(0, 0),
        glm::vec2(m_viewport->getWidth(), m_viewport->getHeight()),
        m_viewport->getViewMatrix(),
        m_viewport->getProjectionMatrix()
    );

    // Get properties of seed object
    auto seedObject = m_smartSelectionState.seedObjects[0];
    bool isMesh = seedObject->hasMeshData();
    auto seedTransform = seedObject->getWorldTransform();
    glm::vec3 seedScale(
        glm::length(seedTransform[0]),
        glm::length(seedTransform[1]),
        glm::length(seedTransform[2])
    );
    glm::vec3 seedPosition(seedTransform[3]);

    // Find similar objects
    for (const auto& obj : allObjects) {
        if (obj == seedObject) continue;

        // Check object type
        if (obj->hasMeshData() != isMesh) continue;

        // Check scale similarity
        auto objTransform = obj->getWorldTransform();
        glm::vec3 objScale(
            glm::length(objTransform[0]),
            glm::length(objTransform[1]),
            glm::length(objTransform[2])
        );
        float scaleDiff = glm::length(objScale - seedScale) / glm::length(seedScale);
        if (scaleDiff > m_smartSelectionState.similarityThreshold) continue;

        // Check spatial proximity if enabled
        if (m_smartSelectionState.autoComplete) {
            glm::vec3 objPosition(objTransform[3]);
            float distance = glm::length(objPosition - seedPosition);
            float maxDist = glm::length(seedScale) * 5.0f; // Adjust proximity threshold
            if (distance > maxDist) continue;
        }

        similarObjects.push_back(obj);
    }

    // Add seed object to results
    similarObjects.push_back(seedObject);

    return similarObjects;
}

void SelectionInteractionManager::updatePreview() {
    if (m_previewCallback && m_config.previewSelection) {
        auto filtered = applyFilters(m_selectionState.preview);
        m_previewCallback(filtered);
    }
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::applyFilters(
    const std::vector<SceneNode::Ptr>& selection
) const {
    return m_filterManager->applyFilters(selection);
}

void SelectionInteractionManager::updateFeedback() {
    if (!m_feedbackCallback || !m_config.visualFeedback) return;

    std::vector<glm::vec2> feedback;

    switch (m_currentMode) {
        case SelectionMode::Box:
            {
                float x1 = m_selectionState.startX;
                float y1 = m_selectionState.startY;
                float x2 = m_selectionState.currentX;
                float y2 = m_selectionState.currentY;
                feedback = {
                    glm::vec2(x1, y1),
                    glm::vec2(x2, y1),
                    glm::vec2(x2, y2),
                    glm::vec2(x1, y2),
                    glm::vec2(x1, y1)
                };
            }
            break;

        case SelectionMode::Lasso:
            feedback = m_selectionState.points;
            if (feedback.size() > 1) {
                feedback.push_back(feedback.front()); // Close the loop
            }
            break;

        case SelectionMode::Paint:
            feedback = m_selectionState.points;
            break;

        default:
            break;
    }

    m_feedbackCallback(feedback);
}

std::vector<SceneNode::Ptr> SelectionInteractionManager::applyModifier(
    const std::vector<SceneNode::Ptr>& newSelection
) {
    switch (m_currentModifier) {
        case SelectionModifier::None:
            return newSelection;

        case SelectionModifier::Add:
            {
                std::unordered_set<SceneNode::Ptr> combined(
                    m_currentSelection.begin(),
                    m_currentSelection.end()
                );
                combined.insert(newSelection.begin(), newSelection.end());
                return std::vector<SceneNode::Ptr>(combined.begin(), combined.end());
            }

        case SelectionModifier::Subtract:
            {
                std::unordered_set<SceneNode::Ptr> remaining(
                    m_currentSelection.begin(),
                    m_currentSelection.end()
                );
                for (const auto& node : newSelection) {
                    remaining.erase(node);
                }
                return std::vector<SceneNode::Ptr>(remaining.begin(), remaining.end());
            }

        case SelectionModifier::Intersect:
            {
                std::unordered_set<SceneNode::Ptr> newSet(
                    newSelection.begin(),
                    newSelection.end()
                );
                std::vector<SceneNode::Ptr> intersection;
                for (const auto& node : m_currentSelection) {
                    if (newSet.find(node) != newSet.end()) {
                        intersection.push_back(node);
                    }
                }
                return intersection;
            }

        default:
            return newSelection;
    }
}

bool SelectionInteractionManager::isValidSelection() const {
    return m_selectionState.active && !m_selectionState.points.empty();
}

void SelectionInteractionManager::clearSelectionState() {
    m_selectionState.active = false;
    m_selectionState.points.clear();
    m_selectionState.preview.clear();
    m_smartSelectionState.seedObjects.clear();
}

} // namespace Graphics
} // namespace RebelCAD
