#include "graphics/ViewportLayoutManager.h"
#include <fstream>
#include <algorithm>

namespace RebelCAD {
namespace Graphics {

ViewportLayoutManager::ViewportLayoutManager() {
    // Initialize with standard templates
    createStandardTemplate(StandardLayoutType::Single, "Single View");
    createStandardTemplate(StandardLayoutType::Double, "Double View");
    createStandardTemplate(StandardLayoutType::Triple, "Triple View");
    createStandardTemplate(StandardLayoutType::Quad, "Quad View");
    createStandardTemplate(StandardLayoutType::ThreeByTwo, "3x2 Grid");
    createStandardTemplate(StandardLayoutType::TwoByThree, "2x3 Grid");
}

bool ViewportLayoutManager::createTemplate(
    const std::string& name,
    const std::vector<ViewportLayout>& viewports,
    const glm::vec2& aspectRatio,
    bool maintainAspectRatio
) {
    if (m_templates.find(name) != m_templates.end()) {
        return false; // Template already exists
    }

    LayoutTemplate templ{
        .name = name,
        .viewports = viewports,
        .aspectRatio = aspectRatio,
        .maintainAspectRatio = maintainAspectRatio
    };

    m_templates[name] = std::move(templ);
    return true;
}

std::optional<LayoutTemplate> ViewportLayoutManager::loadTemplate(
    const std::string& name
) const {
    auto it = m_templates.find(name);
    if (it != m_templates.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool ViewportLayoutManager::saveViewportStates(
    const std::string& name,
    const std::vector<std::shared_ptr<Viewport>>& viewports
) {
    auto it = m_templates.find(name);
    if (it == m_templates.end() || viewports.size() != it->second.viewports.size()) {
        return false;
    }

    // Save state for each viewport
    for (size_t i = 0; i < viewports.size(); ++i) {
        const auto& viewport = viewports[i];
        ViewState state;
        state.position = viewport->getEyePosition();
        state.target = viewport->getTarget();
        state.upVector = glm::vec3(0, 1, 0); // Default up vector
        state.projection = viewport->getProjectionType();
        state.fieldOfView = 45.0f; // Default FOV
        state.orthoScale = viewport->getOrthoScale();
        state.nearPlane = viewport->getNearPlane();
        state.farPlane = viewport->getFarPlane();

        it->second.viewports[i].savedState = state;
    }

    return true;
}

bool ViewportLayoutManager::loadViewportStates(
    const std::string& name,
    std::vector<std::shared_ptr<Viewport>>& viewports
) {
    auto it = m_templates.find(name);
    if (it == m_templates.end() || viewports.size() != it->second.viewports.size()) {
        return false;
    }

    // Apply saved state to each viewport
    for (size_t i = 0; i < viewports.size(); ++i) {
        const auto& layout = it->second.viewports[i];
        if (!layout.savedState) continue;

        const auto& state = *layout.savedState;
        auto& viewport = viewports[i];

        if (state.projection == ViewportProjection::Perspective) {
            viewport->setPerspective(
                state.fieldOfView,
                viewport->getAspectRatio(),
                state.nearPlane,
                state.farPlane
            );
        } else {
            float scale = state.orthoScale;
            float aspect = viewport->getAspectRatio();
            viewport->setOrthographic(
                -scale * aspect,
                scale * aspect,
                -scale,
                scale,
                state.nearPlane,
                state.farPlane
            );
        }

        viewport->setView(state.position, state.target, state.upVector);
    }

    return true;
}

bool ViewportLayoutManager::createStandardTemplate(
    StandardLayoutType type,
    const std::string& name
) {
    std::vector<ViewportLayout> layouts;
    std::string templateName = name.empty() ? getDefaultTemplateName(type) : name;

    switch (type) {
        case StandardLayoutType::Single:
            layouts = generateSingleLayout();
            break;
        case StandardLayoutType::Double:
            layouts = generateDoubleLayout();
            break;
        case StandardLayoutType::Triple:
            layouts = generateTripleLayout();
            break;
        case StandardLayoutType::Quad:
            layouts = generateQuadLayout();
            break;
        case StandardLayoutType::ThreeByTwo:
            layouts = generate3x2Layout();
            break;
        case StandardLayoutType::TwoByThree:
            layouts = generate2x3Layout();
            break;
    }

    return createTemplate(templateName, layouts);
}

bool ViewportLayoutManager::applyTemplate(
    const std::string& templateName,
    std::vector<std::shared_ptr<Viewport>>& viewports,
    const glm::vec2& containerSize
) {
    auto templ = loadTemplate(templateName);
    if (!templ) return false;

    // Resize viewport vector if needed
    viewports.resize(templ->viewports.size());

    // Apply layout to each viewport
    for (size_t i = 0; i < templ->viewports.size(); ++i) {
        const auto& layout = templ->viewports[i];
        auto& viewport = viewports[i];

        if (!viewport) {
            viewport = std::make_shared<Viewport>();
        }

        // Calculate pixel coordinates
        glm::vec2 pos = calculatePixelPosition(layout.position, containerSize);
        glm::vec2 size = calculatePixelSize(layout.size, containerSize);

        // Apply viewport settings
        viewport->setViewport(
            static_cast<int>(pos.x),
            static_cast<int>(pos.y),
            static_cast<int>(size.x),
            static_cast<int>(size.y)
        );

        // Apply saved state if available
        if (layout.savedState) {
            const auto& state = *layout.savedState;
            if (state.projection == ViewportProjection::Perspective) {
                viewport->setPerspective(
                    state.fieldOfView,
                    size.x / size.y,
                    state.nearPlane,
                    state.farPlane
                );
            } else {
                float scale = state.orthoScale;
                float aspect = size.x / size.y;
                viewport->setOrthographic(
                    -scale * aspect,
                    scale * aspect,
                    -scale,
                    scale,
                    state.nearPlane,
                    state.farPlane
                );
            }
            viewport->setView(state.position, state.target, state.upVector);
        } else {
            // Apply default projection
            if (layout.defaultProjection == ViewportProjection::Perspective) {
                viewport->setPerspective(
                    45.0f,
                    size.x / size.y,
                    0.1f,
                    1000.0f
                );
            } else {
                float scale = 5.0f;
                float aspect = size.x / size.y;
                viewport->setOrthographic(
                    -scale * aspect,
                    scale * aspect,
                    -scale,
                    scale,
                    0.1f,
                    1000.0f
                );
            }
        }
    }

    return true;
}

std::vector<std::string> ViewportLayoutManager::listTemplates() const {
    std::vector<std::string> names;
    names.reserve(m_templates.size());
    for (const auto& [name, _] : m_templates) {
        names.push_back(name);
    }
    return names;
}

bool ViewportLayoutManager::removeTemplate(const std::string& name) {
    return m_templates.erase(name) > 0;
}

bool ViewportLayoutManager::saveTemplates(const std::string& filepath) const {
    try {
        nlohmann::json json;
        for (const auto& [name, templ] : m_templates) {
            json[name] = serializeTemplate(templ);
        }

        std::ofstream file(filepath);
        file << json.dump(2);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ViewportLayoutManager::loadTemplates(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        nlohmann::json json;
        file >> json;

        m_templates.clear();
        for (const auto& [name, templateJson] : json.items()) {
            m_templates[name] = deserializeTemplate(templateJson);
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// Private methods

std::vector<ViewportLayout> ViewportLayoutManager::generateSingleLayout() {
    return {
        ViewportLayout{
            .position = glm::vec2(0.0f),
            .size = glm::vec2(1.0f),
            .defaultProjection = ViewportProjection::Perspective,
            .name = "Main View"
        }
    };
}

std::vector<ViewportLayout> ViewportLayoutManager::generateDoubleLayout() {
    return {
        ViewportLayout{
            .position = glm::vec2(0.0f, 0.0f),
            .size = glm::vec2(0.5f, 1.0f),
            .defaultProjection = ViewportProjection::Perspective,
            .name = "Left View"
        },
        ViewportLayout{
            .position = glm::vec2(0.5f, 0.0f),
            .size = glm::vec2(0.5f, 1.0f),
            .defaultProjection = ViewportProjection::Perspective,
            .name = "Right View"
        }
    };
}

std::vector<ViewportLayout> ViewportLayoutManager::generateTripleLayout() {
    return {
        ViewportLayout{
            .position = glm::vec2(0.0f, 0.0f),
            .size = glm::vec2(0.5f, 1.0f),
            .defaultProjection = ViewportProjection::Perspective,
            .name = "Main View"
        },
        ViewportLayout{
            .position = glm::vec2(0.5f, 0.0f),
            .size = glm::vec2(0.5f, 0.5f),
            .defaultProjection = ViewportProjection::Orthographic,
            .name = "Top View"
        },
        ViewportLayout{
            .position = glm::vec2(0.5f, 0.5f),
            .size = glm::vec2(0.5f, 0.5f),
            .defaultProjection = ViewportProjection::Orthographic,
            .name = "Front View"
        }
    };
}

std::vector<ViewportLayout> ViewportLayoutManager::generateQuadLayout() {
    return {
        ViewportLayout{
            .position = glm::vec2(0.0f, 0.5f),
            .size = glm::vec2(0.5f, 0.5f),
            .defaultProjection = ViewportProjection::Perspective,
            .name = "Perspective"
        },
        ViewportLayout{
            .position = glm::vec2(0.5f, 0.5f),
            .size = glm::vec2(0.5f, 0.5f),
            .defaultProjection = ViewportProjection::Orthographic,
            .name = "Top"
        },
        ViewportLayout{
            .position = glm::vec2(0.0f, 0.0f),
            .size = glm::vec2(0.5f, 0.5f),
            .defaultProjection = ViewportProjection::Orthographic,
            .name = "Front"
        },
        ViewportLayout{
            .position = glm::vec2(0.5f, 0.0f),
            .size = glm::vec2(0.5f, 0.5f),
            .defaultProjection = ViewportProjection::Orthographic,
            .name = "Right"
        }
    };
}

std::vector<ViewportLayout> ViewportLayoutManager::generate3x2Layout() {
    std::vector<ViewportLayout> layouts;
    const float width = 1.0f / 3.0f;
    const float height = 0.5f;

    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 3; ++col) {
            layouts.push_back(ViewportLayout{
                .position = glm::vec2(col * width, row * height),
                .size = glm::vec2(width, height),
                .defaultProjection = ViewportProjection::Perspective,
                .name = "View " + std::to_string(row * 3 + col + 1)
            });
        }
    }

    return layouts;
}

std::vector<ViewportLayout> ViewportLayoutManager::generate2x3Layout() {
    std::vector<ViewportLayout> layouts;
    const float width = 0.5f;
    const float height = 1.0f / 3.0f;

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 2; ++col) {
            layouts.push_back(ViewportLayout{
                .position = glm::vec2(col * width, row * height),
                .size = glm::vec2(width, height),
                .defaultProjection = ViewportProjection::Perspective,
                .name = "View " + std::to_string(row * 2 + col + 1)
            });
        }
    }

    return layouts;
}

glm::vec2 ViewportLayoutManager::calculatePixelPosition(
    const glm::vec2& normalizedPos,
    const glm::vec2& containerSize
) const {
    return normalizedPos * containerSize;
}

glm::vec2 ViewportLayoutManager::calculatePixelSize(
    const glm::vec2& normalizedSize,
    const glm::vec2& containerSize
) const {
    return normalizedSize * containerSize;
}

nlohmann::json ViewportLayoutManager::serializeTemplate(
    const LayoutTemplate& templ
) const {
    nlohmann::json json;
    json["name"] = templ.name;
    json["aspectRatio"] = {templ.aspectRatio.x, templ.aspectRatio.y};
    json["maintainAspectRatio"] = templ.maintainAspectRatio;

    nlohmann::json viewportsJson = nlohmann::json::array();
    for (const auto& viewport : templ.viewports) {
        nlohmann::json viewportJson;
        viewportJson["position"] = {viewport.position.x, viewport.position.y};
        viewportJson["size"] = {viewport.size.x, viewport.size.y};
        viewportJson["defaultProjection"] = static_cast<int>(viewport.defaultProjection);
        viewportJson["name"] = viewport.name;

        if (viewport.savedState) {
            viewportJson["savedState"] = serializeViewState(*viewport.savedState);
        }

        viewportsJson.push_back(viewportJson);
    }
    json["viewports"] = viewportsJson;

    return json;
}

LayoutTemplate ViewportLayoutManager::deserializeTemplate(
    const nlohmann::json& json
) const {
    LayoutTemplate templ;
    templ.name = json["name"];
    templ.aspectRatio = glm::vec2(
        json["aspectRatio"][0],
        json["aspectRatio"][1]
    );
    templ.maintainAspectRatio = json["maintainAspectRatio"];

    for (const auto& viewportJson : json["viewports"]) {
        ViewportLayout layout;
        layout.position = glm::vec2(
            viewportJson["position"][0],
            viewportJson["position"][1]
        );
        layout.size = glm::vec2(
            viewportJson["size"][0],
            viewportJson["size"][1]
        );
        layout.defaultProjection = static_cast<ViewportProjection>(
            viewportJson["defaultProjection"].get<int>()
        );
        layout.name = viewportJson["name"];

        if (viewportJson.contains("savedState")) {
            layout.savedState = deserializeViewState(viewportJson["savedState"]);
        }

        templ.viewports.push_back(layout);
    }

    return templ;
}

nlohmann::json ViewportLayoutManager::serializeViewState(
    const ViewState& state
) const {
    nlohmann::json json;
    json["position"] = {state.position.x, state.position.y, state.position.z};
    json["target"] = {state.target.x, state.target.y, state.target.z};
    json["upVector"] = {state.upVector.x, state.upVector.y, state.upVector.z};
    json["projection"] = static_cast<int>(state.projection);
    json["fieldOfView"] = state.fieldOfView;
    json["orthoScale"] = state.orthoScale;
    json["nearPlane"] = state.nearPlane;
    json["farPlane"] = state.farPlane;
    return json;
}

ViewState ViewportLayoutManager::deserializeViewState(
    const nlohmann::json& json
) const {
    ViewState state;
    state.position = glm::vec3(
        json["position"][0],
        json["position"][1],
        json["position"][2]
    );
    state.target = glm::vec3(
        json["target"][0],
        json["target"][1],
        json["target"][2]
    );
    state.upVector = glm::vec3(
        json["upVector"][0],
        json["upVector"][1],
        json["upVector"][2]
    );
    state.projection = static_cast<ViewportProjection>(
        json["projection"].get<int>()
    );
    state.fieldOfView = json["fieldOfView"];
    state.orthoScale = json["orthoScale"];
    state.nearPlane = json["nearPlane"];
    state.farPlane = json["farPlane"];
    return state;
}

std::string ViewportLayoutManager::getDefaultTemplateName(
    StandardLayoutType type
) const {
    switch (type) {
        case StandardLayoutType::Single: return "Single View";
        case StandardLayoutType::Double: return "Double View";
        case StandardLayoutType::Triple: return "Triple View";
        case StandardLayoutType::Quad: return "Quad View";
        case StandardLayoutType::ThreeByTwo: return "3x2 Grid";
        case StandardLayoutType::TwoByThree: return "2x3 Grid";
        default: return "Custom Layout";
    }
}

} // namespace Graphics
} // namespace RebelCAD
