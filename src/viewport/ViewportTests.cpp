#include <gtest/gtest.h>
#include "graphics/Viewport.h"
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/matrix_query.hpp>

using namespace rebel_cad::graphics;

class ViewportTest : public ::testing::Test {
protected:
    const float EPSILON = 0.0001f;
    const int WIDTH = 800;
    const int HEIGHT = 600;
    
    Viewport viewport{WIDTH, HEIGHT};
};

TEST_F(ViewportTest, DefaultConstructorState) {
    // Check default camera position
    EXPECT_NEAR(glm::length(viewport.getCameraPosition() - glm::vec3(0.0f, 0.0f, 10.0f)), 0.0f, EPSILON);
    
    // Check default target position
    EXPECT_NEAR(glm::length(viewport.getCameraTarget()), 0.0f, EPSILON);
    
    // Check view matrix is not identity
    EXPECT_FALSE(viewport.getViewMatrix() == glm::mat4(1.0f));
    
    // Check projection matrix is not identity
    EXPECT_FALSE(viewport.getProjectionMatrix() == glm::mat4(1.0f));
}

TEST_F(ViewportTest, ProjectionMatrixAspectRatio) {
    glm::mat4 proj = viewport.getProjectionMatrix();
    float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    
    // For perspective projection, check if aspect ratio is maintained
    EXPECT_NEAR(proj[1][1] * aspect, proj[0][0], EPSILON);
    
    // Test with different dimensions
    viewport.setDimensions(1024, 768);
    proj = viewport.getProjectionMatrix();
    aspect = 1024.0f / 768.0f;
    EXPECT_NEAR(proj[1][1] * aspect, proj[0][0], EPSILON);
}

TEST_F(ViewportTest, StandardViews) {
    const float distance = 10.0f;
    
    // Test Front view
    viewport.setStandardView(ViewportView::Front);
    EXPECT_NEAR(viewport.getCameraPosition().z, distance, EPSILON);
    EXPECT_NEAR(glm::length(viewport.getCameraTarget()), 0.0f, EPSILON);
    
    // Test Top view
    viewport.setStandardView(ViewportView::Top);
    EXPECT_NEAR(viewport.getCameraPosition().y, distance, EPSILON);
    EXPECT_NEAR(glm::length(viewport.getCameraTarget()), 0.0f, EPSILON);
    
    // Test Isometric view
    viewport.setStandardView(ViewportView::Isometric);
    float isoDistance = distance * 0.577f;
    EXPECT_NEAR(viewport.getCameraPosition().x, isoDistance, EPSILON);
    EXPECT_NEAR(viewport.getCameraPosition().y, isoDistance, EPSILON);
    EXPECT_NEAR(viewport.getCameraPosition().z, isoDistance, EPSILON);
}

TEST_F(ViewportTest, ProjectionModeSwitch) {
    // Get initial projection matrix
    glm::mat4 perspMatrix = viewport.getProjectionMatrix();
    
    // Switch to orthographic
    viewport.setProjection(ViewportProjection::Orthographic);
    glm::mat4 orthoMatrix = viewport.getProjectionMatrix();
    
    // Matrices should be different
    EXPECT_FALSE(perspMatrix == orthoMatrix);
    
    // Switch back to perspective
    viewport.setProjection(ViewportProjection::Perspective);
    glm::mat4 newPerspMatrix = viewport.getProjectionMatrix();
    
    // Should match original perspective matrix
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            EXPECT_NEAR(perspMatrix[i][j], newPerspMatrix[i][j], EPSILON);
        }
    }
}

TEST_F(ViewportTest, CameraManipulation) {
    glm::vec3 initialPos = viewport.getCameraPosition();
    glm::vec3 initialTarget = viewport.getCameraTarget();
    
    // Test pan
    viewport.pan(10.0f, 0.0f);
    EXPECT_NE(viewport.getCameraPosition(), initialPos);
    EXPECT_NE(viewport.getCameraTarget(), initialTarget);
    
    // Test zoom
    initialPos = viewport.getCameraPosition();
    viewport.zoom(2.0f);
    EXPECT_NEAR(glm::length(viewport.getCameraPosition() - viewport.getCameraTarget()),
                glm::length(initialPos - viewport.getCameraTarget()) / 2.0f,
                EPSILON);
    
    // Test rotate
    initialPos = viewport.getCameraPosition();
    viewport.rotate(glm::pi<float>() / 2.0f, 0.0f);
    EXPECT_NE(viewport.getCameraPosition(), initialPos);
    EXPECT_NEAR(glm::length(viewport.getCameraPosition() - viewport.getCameraTarget()),
                glm::length(initialPos - viewport.getCameraTarget()),
                EPSILON);
}

TEST_F(ViewportTest, ViewportSynchronization) {
    Viewport otherViewport(WIDTH, HEIGHT);
    
    // Initially not synchronized
    EXPECT_FALSE(viewport.isSynchronizedWith(otherViewport));
    EXPECT_FALSE(otherViewport.isSynchronizedWith(viewport));
    
    // Synchronize viewports
    viewport.synchronizeWith(otherViewport);
    EXPECT_TRUE(viewport.isSynchronizedWith(otherViewport));
    EXPECT_TRUE(otherViewport.isSynchronizedWith(viewport));
    
    // Test synchronization of camera movements
    glm::vec3 initialPos = viewport.getCameraPosition();
    viewport.pan(10.0f, 0.0f);
    EXPECT_EQ(viewport.getCameraPosition(), otherViewport.getCameraPosition());
    EXPECT_EQ(viewport.getCameraTarget(), otherViewport.getCameraTarget());
    
    // Unsynchronize viewports
    viewport.synchronizeWith(otherViewport, false);
    EXPECT_FALSE(viewport.isSynchronizedWith(otherViewport));
    EXPECT_FALSE(otherViewport.isSynchronizedWith(viewport));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
