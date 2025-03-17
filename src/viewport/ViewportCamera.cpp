#include "graphics/ViewportCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace RebelCAD {
namespace Graphics {

ViewportCamera::ViewportCamera()
    : position_(0.0f, 0.0f, 5.0f)
    , target_(0.0f, 0.0f, 0.0f)
    , up_(0.0f, 1.0f, 0.0f)
    , fov_(glm::radians(45.0f))
    , aspect_(1.0f)
    , nearPlane_(0.1f)
    , farPlane_(1000.0f)
    , viewDirty_(true)
    , projDirty_(true) {
    updateFrustum();
}

void ViewportCamera::setPosition(const glm::vec3& position) {
    position_ = position;
    viewDirty_ = true;
}

void ViewportCamera::setTarget(const glm::vec3& target) {
    target_ = target;
    viewDirty_ = true;
}

void ViewportCamera::setUp(const glm::vec3& up) {
    up_ = glm::normalize(up);
    viewDirty_ = true;
}

void ViewportCamera::setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    fov_ = fov;
    aspect_ = aspect;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
    projDirty_ = true;
}

glm::mat4 ViewportCamera::getViewMatrix() const {
    updateViewMatrix();
    return viewMatrix_;
}

glm::mat4 ViewportCamera::getProjectionMatrix() const {
    updateProjMatrix();
    return projMatrix_;
}

glm::mat4 ViewportCamera::getViewProjectionMatrix() const {
    updateViewMatrix();
    updateProjMatrix();
    return projMatrix_ * viewMatrix_;
}

glm::vec3 ViewportCamera::getForward() const {
    return -glm::normalize(position_ - target_);
}

glm::vec3 ViewportCamera::getRight() const {
    return glm::normalize(glm::cross(getForward(), up_));
}

float ViewportCamera::distanceTo(const glm::vec3& point) const {
    return glm::length(point - position_);
}

float ViewportCamera::distanceToSphere(const glm::vec3& center, float radius) const {
    return glm::max(0.0f, distanceTo(center) - radius);
}

void ViewportCamera::updateFrustum() {
    updateViewMatrix();
    updateProjMatrix();

    // Get view-projection matrix
    glm::mat4 viewProj = projMatrix_ * viewMatrix_;

    // Extract frustum planes from view-projection matrix
    // Left plane
    frustumPlanes_[0].equation.x = viewProj[0][3] + viewProj[0][0];
    frustumPlanes_[0].equation.y = viewProj[1][3] + viewProj[1][0];
    frustumPlanes_[0].equation.z = viewProj[2][3] + viewProj[2][0];
    frustumPlanes_[0].equation.w = viewProj[3][3] + viewProj[3][0];

    // Right plane
    frustumPlanes_[1].equation.x = viewProj[0][3] - viewProj[0][0];
    frustumPlanes_[1].equation.y = viewProj[1][3] - viewProj[1][0];
    frustumPlanes_[1].equation.z = viewProj[2][3] - viewProj[2][0];
    frustumPlanes_[1].equation.w = viewProj[3][3] - viewProj[3][0];

    // Bottom plane
    frustumPlanes_[2].equation.x = viewProj[0][3] + viewProj[0][1];
    frustumPlanes_[2].equation.y = viewProj[1][3] + viewProj[1][1];
    frustumPlanes_[2].equation.z = viewProj[2][3] + viewProj[2][1];
    frustumPlanes_[2].equation.w = viewProj[3][3] + viewProj[3][1];

    // Top plane
    frustumPlanes_[3].equation.x = viewProj[0][3] - viewProj[0][1];
    frustumPlanes_[3].equation.y = viewProj[1][3] - viewProj[1][1];
    frustumPlanes_[3].equation.z = viewProj[2][3] - viewProj[2][1];
    frustumPlanes_[3].equation.w = viewProj[3][3] - viewProj[3][1];

    // Near plane
    frustumPlanes_[4].equation.x = viewProj[0][3] + viewProj[0][2];
    frustumPlanes_[4].equation.y = viewProj[1][3] + viewProj[1][2];
    frustumPlanes_[4].equation.z = viewProj[2][3] + viewProj[2][2];
    frustumPlanes_[4].equation.w = viewProj[3][3] + viewProj[3][2];

    // Far plane
    frustumPlanes_[5].equation.x = viewProj[0][3] - viewProj[0][2];
    frustumPlanes_[5].equation.y = viewProj[1][3] - viewProj[1][2];
    frustumPlanes_[5].equation.z = viewProj[2][3] - viewProj[2][2];
    frustumPlanes_[5].equation.w = viewProj[3][3] - viewProj[3][2];

    // Normalize all planes
    for (auto& plane : frustumPlanes_) {
        float length = glm::length(glm::vec3(plane.equation));
        plane.equation /= length;
    }
}

bool ViewportCamera::isPointVisible(const glm::vec3& point) const {
    // Test point against all frustum planes
    for (const auto& plane : frustumPlanes_) {
        if (glm::dot(glm::vec4(point, 1.0f), plane.equation) < 0) {
            return false;
        }
    }
    return true;
}

bool ViewportCamera::isSphereVisible(const glm::vec3& center, float radius) const {
    // Test sphere against all frustum planes
    for (const auto& plane : frustumPlanes_) {
        float distance = glm::dot(glm::vec4(center, 1.0f), plane.equation);
        if (distance < -radius) {
            return false;
        }
    }
    return true;
}

void ViewportCamera::updateViewMatrix() const {
    if (!viewDirty_) return;

    viewMatrix_ = glm::lookAt(position_, target_, up_);
    viewDirty_ = false;
}

void ViewportCamera::updateProjMatrix() const {
    if (!projDirty_) return;

    projMatrix_ = glm::perspective(fov_, aspect_, nearPlane_, farPlane_);
    projDirty_ = false;
}

} // namespace Graphics
} // namespace RebelCAD
