#include "Graphics/Viewport.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RebelCAD {
namespace Graphics {

Viewport::Viewport()
    : x_(0)
    , y_(0)
    , width_(800)
    , height_(600)
    , fov_(glm::radians(45.0f))
    , aspectRatio_(800.0f / 600.0f)
    , nearPlane_(0.1f)
    , farPlane_(1000.0f)
    , orthoScale_(5.0f)
    , eyePosition_(0.0f, 0.0f, 10.0f)
    , target_(0.0f)
    , up_(0.0f, 1.0f, 0.0f)
    , projectionType_(ViewportProjection::Perspective)
    , viewMatrix_(1.0f)
    , projectionMatrix_(1.0f)
    , viewProjectionMatrix_(1.0f) {
    
    // Initialize matrices
    setPerspective(fov_, aspectRatio_, nearPlane_, farPlane_);
    setView(eyePosition_, target_, up_);
}

void Viewport::setViewport(int x, int y, int width, int height) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    aspectRatio_ = static_cast<float>(width) / static_cast<float>(height);
    
    // Update projection matrix with new aspect ratio
    if (projectionType_ == ViewportProjection::Perspective) {
        setPerspective(fov_, aspectRatio_, nearPlane_, farPlane_);
    } else {
        float halfWidth = orthoScale_ * aspectRatio_;
        float halfHeight = orthoScale_;
        setOrthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane_, farPlane_);
    }
}

void Viewport::setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    fov_ = fov;
    aspectRatio_ = aspect;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
    projectionType_ = ViewportProjection::Perspective;
    
    projectionMatrix_ = glm::perspective(fov_, aspectRatio_, nearPlane_, farPlane_);
    updateViewProjectionMatrix();
}

void Viewport::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
    projectionType_ = ViewportProjection::Orthographic;
    
    // Calculate ortho scale from the provided dimensions
    orthoScale_ = (top - bottom) * 0.5f;
    
    projectionMatrix_ = glm::ortho(left, right, bottom, top, nearPlane_, farPlane_);
    updateViewProjectionMatrix();
}

void Viewport::setView(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up) {
    eyePosition_ = eye;
    target_ = target;
    up_ = up;
    
    viewMatrix_ = glm::lookAt(eyePosition_, target_, up_);
    updateViewProjectionMatrix();
}

Viewport::Ray Viewport::screenToRay(const glm::vec2& screenPos) const {
    // Convert screen coordinates to normalized device coordinates (-1 to 1)
    float ndcX = (screenPos.x - x_) / width_ * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenPos.y - y_) / height_ * 2.0f; // Flip Y for OpenGL
    
    // Create ray in clip space
    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    
    // Transform to eye space
    glm::vec4 rayEye = glm::inverse(projectionMatrix_) * rayClip;
    rayEye.z = -1.0f;
    rayEye.w = 0.0f;
    
    // Transform to world space
    glm::vec4 rayWorld = glm::inverse(viewMatrix_) * rayEye;
    
    Ray ray;
    ray.origin = eyePosition_;
    ray.direction = glm::normalize(glm::vec3(rayWorld));
    
    return ray;
}

Frustum Viewport::screenRectToFrustum(const glm::vec2& rectMin, const glm::vec2& rectMax) const {
    // Convert screen rectangle corners to rays
    Ray topLeft = screenToRay(glm::vec2(rectMin.x, rectMin.y));
    Ray topRight = screenToRay(glm::vec2(rectMax.x, rectMin.y));
    Ray bottomLeft = screenToRay(glm::vec2(rectMin.x, rectMax.y));
    Ray bottomRight = screenToRay(glm::vec2(rectMax.x, rectMax.y));
    
    // Create frustum planes from rays
    glm::mat4 frustumMatrix(1.0f);
    
    // Near and far planes
    float nearDist = nearPlane_;
    float farDist = farPlane_;
    
    // Calculate frustum corners
    glm::vec3 ntl = eyePosition_ + topLeft.direction * nearDist;
    glm::vec3 ntr = eyePosition_ + topRight.direction * nearDist;
    glm::vec3 nbl = eyePosition_ + bottomLeft.direction * nearDist;
    glm::vec3 nbr = eyePosition_ + bottomRight.direction * nearDist;
    
    glm::vec3 ftl = eyePosition_ + topLeft.direction * farDist;
    glm::vec3 ftr = eyePosition_ + topRight.direction * farDist;
    glm::vec3 fbl = eyePosition_ + bottomLeft.direction * farDist;
    glm::vec3 fbr = eyePosition_ + bottomRight.direction * farDist;
    
    // Create view matrix for the frustum
    glm::vec3 frustumTarget = (ntl + ntr + nbl + nbr) * 0.25f;
    glm::mat4 frustumView = glm::lookAt(eyePosition_, frustumTarget, up_);
    
    // Create projection matrix for the frustum
    float frustumFov = glm::acos(glm::dot(topLeft.direction, bottomLeft.direction));
    float frustumAspect = glm::length(ntr - ntl) / glm::length(ntl - nbl);
    glm::mat4 frustumProj = glm::perspective(frustumFov, frustumAspect, nearDist, farDist);
    
    // Combine view and projection
    glm::mat4 frustumViewProj = frustumProj * frustumView;
    
    return Frustum(frustumViewProj);
}

glm::vec2 Viewport::worldToScreen(const glm::vec3& worldPos) const {
    // Transform world position to clip space
    glm::vec4 clipPos = viewProjectionMatrix_ * glm::vec4(worldPos, 1.0f);
    
    // Perform perspective divide
    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;
    
    // Convert to screen coordinates
    return glm::vec2(
        (ndcPos.x * 0.5f + 0.5f) * width_ + x_,
        (0.5f - ndcPos.y * 0.5f) * height_ + y_ // Flip Y for OpenGL
    );
}

void Viewport::updateViewProjectionMatrix() {
    viewProjectionMatrix_ = projectionMatrix_ * viewMatrix_;
}

} // namespace Graphics
} // namespace RebelCAD
