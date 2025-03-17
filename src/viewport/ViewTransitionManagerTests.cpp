#include <gtest/gtest.h>
#include <memory>
#include <glm/gtx/vector_query.hpp>
#include <glm/gtc/epsilon.hpp>
#include "graphics/ViewTransitionManager.h"
#include "graphics/SceneGraph.h"
#include "graphics/CameraCollision.h"

namespace RebelCAD {
namespace Graphics {

// Test fixture with access to transition state
class ViewTransitionManagerTest : public ::testing::Test, protected ViewTransitionManager {
protected:
    ViewTransitionManagerTest()
        : ViewTransitionManager(std::make_shared<SceneGraph>(), std::make_shared<CameraCollision>())
    {
    }

    void SetUp() override {
    }


    // Helper function to create a test view state
    ViewState createTestState(const glm::vec3& pos, const glm::vec3& target) {
        ViewState state;
        state.position = pos;
        state.target = target;
        state.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity rotation
        state.fieldOfView = 45.0f;
        state.projectionType = ViewportProjection::Perspective;
        state.orthographicScale = 1.0f;
        return state;
    }

    // Helper function to create test transition parameters
    TransitionParams createTestParams(float duration = 1.0f, bool usePath = false) {
        TransitionParams params;
        params.duration = duration;
        params.usePathPlanning = usePath;
        params.allowInterruption = true;
        params.priority = 0;
        params.easingFunc = nullptr;
        return params;
    }
};

TEST_F(ViewTransitionManagerTest, StartTransition) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(6.0f, 0.0f, 0.0f));
    TransitionParams params = createTestParams();

    uint64_t id = startTransition(start, end, params);
    EXPECT_GT(id, 0);
}

TEST_F(ViewTransitionManagerTest, CancelTransition) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(6.0f, 0.0f, 0.0f));
    TransitionParams params = createTestParams();

    uint64_t id = startTransition(start, end, params);
    EXPECT_TRUE(cancelTransition(id));
    EXPECT_FALSE(cancelTransition(id)); // Should fail second time
}

TEST_F(ViewTransitionManagerTest, UpdateTransition) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(6.0f, 0.0f, 0.0f));
    TransitionParams params = createTestParams(2.0f); // 2 second duration

    bool completionCalled = false;
    setTransitionCompletedCallback([&completionCalled](uint64_t) {
        completionCalled = true;
    });

    startTransition(start, end, params);
    
    // Update halfway through
    update(1.0f);
    EXPECT_FALSE(completionCalled);

    // Complete the transition
    update(1.0f);
    EXPECT_TRUE(completionCalled);
}

TEST_F(ViewTransitionManagerTest, BezierPathPlanning) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(11.0f, 0.0f, 0.0f));
    
    TransitionParams params = createTestParams(1.0f, true);
    params.pathParams = PathPlanningParams{
        .waypoints = {
            PathWaypoint{
                .position = glm::vec3(3.0f, 2.0f, 0.0f),
                .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                .dwellTime = 0.2f,
                .blendRadius = 0.5f
            },
            PathWaypoint{
                .position = glm::vec3(7.0f, -2.0f, 0.0f),
                .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                .dwellTime = 0.3f,
                .blendRadius = 0.5f
            }
        },
        .pathSmoothing = 0.5f,
        .collisionPadding = 0.1f,
        .optimizationPasses = 3,
        .adaptiveSpeed = true
    };

    std::vector<glm::vec3> positions;
    setProgressCallback([&](uint64_t, float) {
        positions.push_back(getCurrentInterpolatedState().position);
    });

    startTransition(start, end, params);
    
    // Update through waypoints
    for (int i = 0; i < 20; i++) {
        update(0.1f);
    }

    // Verify path goes through waypoints
    bool passesNearWaypoints = false;
    for (const auto& pos : positions) {
        if (glm::distance(pos, glm::vec3(3.0f, 2.0f, 0.0f)) < 0.5f ||
            glm::distance(pos, glm::vec3(7.0f, -2.0f, 0.0f)) < 0.5f) {
            passesNearWaypoints = true;
            break;
        }
    }
    EXPECT_TRUE(passesNearWaypoints);
}

TEST_F(ViewTransitionManagerTest, AccelerationCurves) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(11.0f, 0.0f, 0.0f));
    
    TransitionParams params = createTestParams(2.0f, true);
    params.accelParams = AccelerationParams{
        .initialSpeed = 0.0f,
        .maxSpeed = 2.0f,
        .acceleration = 1.0f,
        .deceleration = 1.0f,
        .adaptToPath = true
    };

    std::vector<float> speeds;
    bool hasAcceleration = false;
    bool hasDeceleration = false;

    startTransition(start, end, params);
    
    // Update and track speed changes
    for (int i = 0; i < 20; i++) {
        float lastSpeed = speeds.empty() ? 0.0f : speeds.back();
        update(0.1f);
        speeds.push_back(getCurrentSpeed());
        
        if (!speeds.empty()) {
            float speedDiff = speeds.back() - lastSpeed;
            if (speedDiff > 0.01f) hasAcceleration = true;
            if (speedDiff < -0.01f) hasDeceleration = true;
        }
    }

    EXPECT_TRUE(hasAcceleration);
    EXPECT_TRUE(hasDeceleration);
}

TEST_F(ViewTransitionManagerTest, UpVectorAndLookAhead) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    start.upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    
    ViewState end = createTestState(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(11.0f, 0.0f, 0.0f));
    end.upVector = glm::vec3(0.0f, 0.0f, 1.0f);
    
    TransitionParams params = createTestParams(1.0f, true);
    params.maintainUpVector = true;
    params.lookAheadEnabled = true;

    bool upVectorMaintained = true;
    bool hasLookAheadPoint = false;

    setProgressCallback([&](uint64_t, float) {
        const auto& state = getCurrentInterpolatedState();
        if (state.upVector && start.upVector) {
            upVectorMaintained &= glm::all(glm::epsilonEqual(*state.upVector, *start.upVector, 0.0001f));
        }
        if (state.lookAheadPoint) {
            hasLookAheadPoint = true;
        }
    });

    startTransition(start, end, params);
    
    for (int i = 0; i < 10; i++) {
        update(0.1f);
    }

    EXPECT_TRUE(upVectorMaintained);
    EXPECT_TRUE(hasLookAheadPoint);
}

TEST_F(ViewTransitionManagerTest, TransitionQueueing) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState mid = createTestState(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(6.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(11.0f, 0.0f, 0.0f));
    
    TransitionParams params1 = createTestParams(1.0f);
    params1.priority = 2; // High priority

    TransitionParams params2 = createTestParams(1.0f);
    params2.priority = 1; // Lower priority
    params2.queueIfBlocked = true;

    // Start high priority transition
    startTransition(start, mid, params1);
    
    // Queue lower priority transition
    uint64_t queuedId = startTransition(mid, end, params2);
    EXPECT_GT(queuedId, 0);

    // Update through first transition
    for (int i = 0; i < 10; i++) {
        update(0.1f);
    }

    // Verify second transition starts
    bool secondTransitionStarted = false;
    setProgressCallback([&](uint64_t id, float) {
        if (id == queuedId) secondTransitionStarted = true;
    });

    update(0.1f);
    EXPECT_TRUE(secondTransitionStarted);
}

TEST_F(ViewTransitionManagerTest, StateBlending) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    start.motionSpeed = 1.0f;
    
    ViewState end = createTestState(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(11.0f, 0.0f, 0.0f));
    end.motionSpeed = 2.0f;
    
    TransitionParams params = createTestParams(1.0f);
    params.blendingRadius = 0.5f;

    std::vector<float> speeds;
    setProgressCallback([&](uint64_t, float) {
        const auto& state = getCurrentInterpolatedState();
        if (state.motionSpeed) {
            speeds.push_back(*state.motionSpeed);
        }
    });

    startTransition(start, end, params);
    
    for (int i = 0; i < 10; i++) {
        update(0.1f);
    }

    // Verify smooth speed transition
    EXPECT_GT(speeds.size(), 0);
    bool hasIntermediate = false;
    for (float speed : speeds) {
        if (speed > 1.0f && speed < 2.0f) {
            hasIntermediate = true;
            break;
        }
    }
    EXPECT_TRUE(hasIntermediate);
}

TEST_F(ViewTransitionManagerTest, TransitionInterruption) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState mid = createTestState(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(6.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(11.0f, 0.0f, 0.0f));
    
    TransitionParams params1 = createTestParams(2.0f);
    params1.allowInterruption = true;

    TransitionParams params2 = createTestParams(1.0f);
    params2.priority = 1; // Higher priority

    // Start first transition
    uint64_t id1 = startTransition(start, mid, params1);
    
    // Update halfway
    update(1.0f);
    float midProgress = getCurrentProgress();
    EXPECT_GT(midProgress, 0.0f);
    EXPECT_LT(midProgress, 1.0f);

    // Interrupt with second transition
    uint64_t id2 = interruptWithNewTransition(end, params2);
    EXPECT_GT(id2, 0);
    EXPECT_NE(id2, id1);

    // Verify first transition was cancelled
    EXPECT_FALSE(isTransitionActive(id1));
}

TEST_F(ViewTransitionManagerTest, WaypointDwellTime) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ViewState end = createTestState(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(11.0f, 0.0f, 0.0f));
    
    TransitionParams params = createTestParams(1.0f, true);
    params.pathParams = PathPlanningParams{
        .waypoints = {
            PathWaypoint{
                .position = glm::vec3(5.0f, 0.0f, 0.0f),
                .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                .dwellTime = 0.5f, // Half second dwell
                .blendRadius = 0.5f
            }
        },
        .pathSmoothing = 0.5f,
        .collisionPadding = 0.1f,
        .optimizationPasses = 3,
        .adaptiveSpeed = true
    };

    startTransition(start, end, params);
    
    // Update until we reach waypoint
    float lastProgress = 0.0f;
    int dwellUpdates = 0;
    
    for (int i = 0; i < 20; i++) {
        update(0.1f);
        float progress = getCurrentProgress();
        
        // If progress hasn't changed, we're dwelling
        if (progress == lastProgress) {
            dwellUpdates++;
        }
        lastProgress = progress;
    }

    // Should have dwelled for ~0.5 seconds (5 updates at 0.1s each)
    EXPECT_GE(dwellUpdates, 4);
}

TEST_F(ViewTransitionManagerTest, ProjectionTypeTransition) {
    ViewState start = createTestState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    start.projectionType = ViewportProjection::Perspective;
    
    ViewState end = createTestState(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(6.0f, 0.0f, 0.0f));
    end.projectionType = ViewportProjection::Orthographic;
    
    TransitionParams params = createTestParams(1.0f);

    ViewportProjection midTransitionType = ViewportProjection::Perspective;
    bool projectionChanged = false;

    setProgressCallback([&](uint64_t, float progress) {
        if (progress >= 0.5f && !projectionChanged) {
            projectionChanged = true;
        }
    });

    startTransition(start, end, params);
    
    // Update to just before midpoint
    update(0.49f);
    EXPECT_FALSE(projectionChanged);

    // Update past midpoint
    update(0.02f);
    EXPECT_TRUE(projectionChanged);
}

} // namespace Graphics
} // namespace RebelCAD
