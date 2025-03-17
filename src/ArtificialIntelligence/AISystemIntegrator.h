#pragma once

#include "../Core/Math.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>

// Forward declarations to avoid circular dependencies
class PathfindingSystem;
class AIController;
class DecisionSystem;
class CombatManager;
class DynamicObstacleAvoidance;

// System status enum
enum class AISubsystemStatus {
    Uninitialized,
    Initializing,
    Running,
    Error,
    ShuttingDown
};

// Performance metrics structure
struct AISystemMetrics {
    double pathfindingTime;
    double obstacleAvoidanceTime;
    double decisionMakingTime;
    double combatUpdateTime;
    double totalUpdateTime;
    size_t activeAgents;
    size_t pathRequests;
    size_t decisionRequests;
    double cpuUsage;
    double memoryUsage;
    size_t totalTasksQueued;
    size_t totalTasksProcessed;
    double averageResponseTime;
};

// Priority levels for AI tasks
enum class AITaskPriority {
    Critical,
    High,
    Medium,
    Low,
    Background
};

// System health status
struct AISystemHealth {
    bool isOverloaded;
    bool needsThrottling;
    std::vector<std::string> activeWarnings;
    std::vector<std::string> recoveryActions;
    double systemStability;
};

// Debug visualization settings
struct AIDebugVisualization {
    bool showPathfinding;
    bool showDecisionTrees;
    bool showBehaviorStates;
    bool showPerformanceMetrics;
    Vector3 debugColor;
    float debugScale;
};

class AISystemIntegrator {
public:
    // Load balancing and priority system
    void SetTaskPriority(const std::string& taskId, AITaskPriority priority);
    bool QueueAITask(const std::string& taskId, std::function<void()> task, AITaskPriority priority);
    void ProcessTaskQueue();
    
    // System health monitoring
    AISystemHealth GetSystemHealth() const;
    void MonitorSystemHealth();
    bool AttemptSystemRecovery();
    void AddHealthWarning(const std::string& warning);
    
    // Performance optimization
    void EnableDynamicThrottling(bool enable);
    void SetPerformanceTargets(double maxCpuUsage, double maxMemoryUsage);
    void AdjustWorkload();
    
    // Debug visualization
    void SetDebugVisualization(const AIDebugVisualization& settings);
    void UpdateDebugDisplay();
    void DrawAIDebugInfo();
    
    AISystemIntegrator();
    ~AISystemIntegrator();

    // Initialize all AI subsystems with error handling
    bool Initialize();
    
    // Get the current status of a specific subsystem
    AISubsystemStatus GetSubsystemStatus(const std::string& subsystemName) const;
    
    // Get the last error message for a subsystem
    std::string GetLastError(const std::string& subsystemName) const;
    
    // Get current performance metrics
    const AISystemMetrics& GetPerformanceMetrics() const { return m_Metrics; }

    // Pathfinding integration
    PathfindingSystem& GetPathfindingSystem() { return *m_PathfindingSystem; }
    
    // Behavior Trees and FSM integration
    AIController& GetAIController() { return *m_AIController; }
    
    // NPC Decision Making integration
    DecisionSystem& GetDecisionSystem() { return *m_DecisionSystem; }
    
    // Combat Behavior integration
    CombatManager& GetCombatManager() { return *m_CombatManager; }
    
    // Dynamic Obstacle Avoidance integration
    DynamicObstacleAvoidance& GetObstacleAvoidance() { return *m_ObstacleAvoidance; }

    // Update all AI systems
    void Update(float deltaTime);

    // System communication methods
    bool RequestPath(const Vector3& start, const Vector3& end, std::vector<Vector3>& outPath);
    bool CheckObstacleAvoidance(const Vector3& position, const Vector3& direction, Vector3& outSafeDirection);
    bool UpdateCombatBehavior(const std::string& entityId, const std::string& behaviorType);
    bool MakeDecision(const std::string& entityId, const std::string& context, std::string& outDecision);
    
    // Register behavior tree callbacks with other systems
    void RegisterBehaviorTreeCallbacks();
    
    // Cleanup and shutdown
    void Shutdown();

private:
    std::unique_ptr<PathfindingSystem> m_PathfindingSystem;
    std::unique_ptr<AIController> m_AIController;
    std::unique_ptr<DecisionSystem> m_DecisionSystem;
    std::unique_ptr<CombatManager> m_CombatManager;
    std::unique_ptr<DynamicObstacleAvoidance> m_ObstacleAvoidance;

    // Status tracking
    std::unordered_map<std::string, AISubsystemStatus> m_SubsystemStatus;
    std::unordered_map<std::string, std::string> m_LastErrors;
    
    // Performance metrics
    AISystemMetrics m_Metrics;
    std::chrono::high_resolution_clock::time_point m_LastUpdateTime;
    
    // Load balancing
    struct AITask {
        std::string id;
        std::function<void()> task;
        AITaskPriority priority;
        std::chrono::high_resolution_clock::time_point queueTime;
    };
    std::vector<AITask> m_TaskQueue;
    bool m_DynamicThrottlingEnabled;
    double m_MaxCpuUsage;
    double m_MaxMemoryUsage;
    
    // System health
    AISystemHealth m_SystemHealth;
    std::chrono::high_resolution_clock::time_point m_LastHealthCheck;
    
    // Debug visualization
    AIDebugVisualization m_DebugSettings;
    bool m_DebugEnabled;

    // Helper methods
    void UpdateMetrics();
    void SetSubsystemStatus(const std::string& subsystem, AISubsystemStatus status);
    void SetSubsystemError(const std::string& subsystem, const std::string& error);
    bool ValidateSubsystemState(const std::string& subsystem) const;
    double CalculateCPUUsage();
    double CalculateMemoryUsage();
};
