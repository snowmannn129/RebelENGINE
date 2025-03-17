#include "AISystemIntegrator.h"
#include "./Pathfinding/PathfindingSystem.h"
#include "./BehaviorTreesAndFSMs/AIController.h"
#include "./NPCDecisionMaking/DecisionSystem.h"
#include "./CombatBehavior/CombatManager.h"
#include "./DynamicObstacleAvoidance/DynamicObstacleAvoidance.h"
#include <iostream>
#include <sstream>

AISystemIntegrator::AISystemIntegrator() :
    m_PathfindingSystem(std::make_unique<PathfindingSystem>()),
    m_AIController(std::make_unique<AIController>()),
    m_DecisionSystem(std::make_unique<DecisionSystem>()),
    m_CombatManager(std::make_unique<CombatManager>()),
    m_ObstacleAvoidance(std::make_unique<DynamicObstacleAvoidance>()),
    m_LastUpdateTime(std::chrono::high_resolution_clock::now()),
    m_DynamicThrottlingEnabled(false),
    m_MaxCpuUsage(80.0),
    m_MaxMemoryUsage(85.0),
    m_DebugEnabled(false),
    m_LastHealthCheck(std::chrono::high_resolution_clock::now()) {
    
    // Initialize status tracking
    m_SubsystemStatus = {
        {"PathfindingSystem", AISubsystemStatus::Uninitialized},
        {"AIController", AISubsystemStatus::Uninitialized},
        {"DecisionSystem", AISubsystemStatus::Uninitialized},
        {"CombatManager", AISubsystemStatus::Uninitialized},
        {"ObstacleAvoidance", AISubsystemStatus::Uninitialized}
    };

    // Initialize metrics
    m_Metrics = {0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0.0, 0.0, 0, 0, 0.0};
    
    // Initialize system health
    m_SystemHealth = {false, false, {}, {}, 1.0};
    
    // Initialize debug visualization settings
    m_DebugSettings = {false, false, false, false, Vector3(0, 1, 0), 1.0f};
}

AISystemIntegrator::~AISystemIntegrator() {
    Shutdown();
}

bool AISystemIntegrator::Initialize() {
    // Initialize all subsystems in the correct order with status tracking
    SetSubsystemStatus("PathfindingSystem", AISubsystemStatus::Initializing);
    if (!m_PathfindingSystem->Initialize()) {
        SetSubsystemError("PathfindingSystem", "Failed to initialize");
        return false;
    }
    SetSubsystemStatus("PathfindingSystem", AISubsystemStatus::Running);

    SetSubsystemStatus("AIController", AISubsystemStatus::Initializing);
    if (!m_AIController->Initialize()) {
        SetSubsystemError("AIController", "Failed to initialize");
        return false;
    }
    SetSubsystemStatus("AIController", AISubsystemStatus::Running);

    SetSubsystemStatus("DecisionSystem", AISubsystemStatus::Initializing);
    if (!m_DecisionSystem->Initialize()) {
        SetSubsystemError("DecisionSystem", "Failed to initialize");
        return false;
    }
    SetSubsystemStatus("DecisionSystem", AISubsystemStatus::Running);

    SetSubsystemStatus("CombatManager", AISubsystemStatus::Initializing);
    if (!m_CombatManager->Initialize()) {
        SetSubsystemError("CombatManager", "Failed to initialize");
        return false;
    }
    SetSubsystemStatus("CombatManager", AISubsystemStatus::Running);

    SetSubsystemStatus("ObstacleAvoidance", AISubsystemStatus::Initializing);
    if (!m_ObstacleAvoidance->Initialize()) {
        SetSubsystemError("ObstacleAvoidance", "Failed to initialize");
        return false;
    }
    SetSubsystemStatus("ObstacleAvoidance", AISubsystemStatus::Running);

    std::cout << "AI System Integration completed successfully" << std::endl;
    return true;
}

void AISystemIntegrator::Update(float deltaTime) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Monitor system health and adjust if needed
    MonitorSystemHealth();
    if (m_DynamicThrottlingEnabled) {
        AdjustWorkload();
    }
    
    // Process queued tasks based on priority
    ProcessTaskQueue();

    // Update systems in the correct order with performance tracking
    if (ValidateSubsystemState("ObstacleAvoidance")) {
        auto start = std::chrono::high_resolution_clock::now();
        m_ObstacleAvoidance->Update(deltaTime);
        auto end = std::chrono::high_resolution_clock::now();
        m_Metrics.obstacleAvoidanceTime = std::chrono::duration<double>(end - start).count();
    }

    if (ValidateSubsystemState("PathfindingSystem")) {
        auto start = std::chrono::high_resolution_clock::now();
        m_PathfindingSystem->Update(deltaTime);
        auto end = std::chrono::high_resolution_clock::now();
        m_Metrics.pathfindingTime = std::chrono::duration<double>(end - start).count();
    }

    if (ValidateSubsystemState("DecisionSystem")) {
        auto start = std::chrono::high_resolution_clock::now();
        m_DecisionSystem->Update(deltaTime);
        auto end = std::chrono::high_resolution_clock::now();
        m_Metrics.decisionMakingTime = std::chrono::duration<double>(end - start).count();
    }

    if (ValidateSubsystemState("AIController")) {
        m_AIController->Update(deltaTime);
    }

    if (ValidateSubsystemState("CombatManager")) {
        auto start = std::chrono::high_resolution_clock::now();
        m_CombatManager->Update(deltaTime);
        auto end = std::chrono::high_resolution_clock::now();
        m_Metrics.combatUpdateTime = std::chrono::duration<double>(end - start).count();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    m_Metrics.totalUpdateTime = std::chrono::duration<double>(endTime - startTime).count();
    
    UpdateMetrics();
}

bool AISystemIntegrator::RequestPath(const Vector3& start, const Vector3& end, std::vector<Vector3>& outPath) {
    if (!ValidateSubsystemState("PathfindingSystem")) {
        return false;
    }
    m_Metrics.pathRequests++;
    return m_PathfindingSystem->FindPath(start, end, outPath);
}

bool AISystemIntegrator::CheckObstacleAvoidance(const Vector3& position, const Vector3& direction, Vector3& outSafeDirection) {
    if (!ValidateSubsystemState("ObstacleAvoidance")) {
        return false;
    }
    return m_ObstacleAvoidance->GetSafeDirection(position, direction, outSafeDirection);
}

bool AISystemIntegrator::UpdateCombatBehavior(const std::string& entityId, const std::string& behaviorType) {
    if (!ValidateSubsystemState("CombatManager")) {
        return false;
    }
    return m_CombatManager->UpdateBehavior(entityId, behaviorType);
}

bool AISystemIntegrator::MakeDecision(const std::string& entityId, const std::string& context, std::string& outDecision) {
    if (!ValidateSubsystemState("DecisionSystem")) {
        return false;
    }
    m_Metrics.decisionRequests++;
    return m_DecisionSystem->EvaluateDecision(entityId, context, outDecision);
}

void AISystemIntegrator::RegisterBehaviorTreeCallbacks() {
    // Register pathfinding callback
    m_AIController->RegisterPathfindingCallback([this](const Vector3& start, const Vector3& end, std::vector<Vector3>& path) {
        return this->RequestPath(start, end, path);
    });

    // Register obstacle avoidance callback
    m_AIController->RegisterObstacleCallback([this](const Vector3& pos, const Vector3& dir, Vector3& safeDir) {
        return this->CheckObstacleAvoidance(pos, dir, safeDir);
    });

    // Register combat behavior callback
    m_AIController->RegisterCombatCallback([this](const std::string& id, const std::string& type) {
        return this->UpdateCombatBehavior(id, type);
    });

    // Register decision making callback
    m_AIController->RegisterDecisionCallback([this](const std::string& id, const std::string& ctx, std::string& decision) {
        return this->MakeDecision(id, ctx, decision);
    });
}

AISubsystemStatus AISystemIntegrator::GetSubsystemStatus(const std::string& subsystemName) const {
    auto it = m_SubsystemStatus.find(subsystemName);
    return it != m_SubsystemStatus.end() ? it->second : AISubsystemStatus::Error;
}

std::string AISystemIntegrator::GetLastError(const std::string& subsystemName) const {
    auto it = m_LastErrors.find(subsystemName);
    return it != m_LastErrors.end() ? it->second : "No error recorded";
}

void AISystemIntegrator::UpdateMetrics() {
    // Update active agents count from all systems
    m_Metrics.activeAgents = 
        m_PathfindingSystem->GetActiveAgentCount() +
        m_CombatManager->GetActiveAgentCount() +
        m_DecisionSystem->GetActiveAgentCount();
        
    // Update CPU and memory usage
    m_Metrics.cpuUsage = CalculateCPUUsage();
    m_Metrics.memoryUsage = CalculateMemoryUsage();
    
    // Update task metrics
    if (m_Metrics.totalTasksProcessed > 0) {
        m_Metrics.averageResponseTime = 
            (m_Metrics.pathfindingTime + m_Metrics.decisionMakingTime + m_Metrics.combatUpdateTime) 
            / m_Metrics.totalTasksProcessed;
    }
    
    // Update debug display if enabled
    if (m_DebugEnabled) {
        UpdateDebugDisplay();
    }
}

void AISystemIntegrator::SetSubsystemStatus(const std::string& subsystem, AISubsystemStatus status) {
    m_SubsystemStatus[subsystem] = status;
    if (status == AISubsystemStatus::Error) {
        std::cerr << "AI Subsystem " << subsystem << " entered error state" << std::endl;
    }
}

void AISystemIntegrator::SetSubsystemError(const std::string& subsystem, const std::string& error) {
    m_LastErrors[subsystem] = error;
    SetSubsystemStatus(subsystem, AISubsystemStatus::Error);
    std::cerr << "AI Subsystem " << subsystem << " error: " << error << std::endl;
}

bool AISystemIntegrator::ValidateSubsystemState(const std::string& subsystem) const {
    auto status = GetSubsystemStatus(subsystem);
    if (status != AISubsystemStatus::Running) {
        std::cerr << "AI Subsystem " << subsystem << " is not in running state. Current state: " 
                  << static_cast<int>(status) << std::endl;
        return false;
    }
    return true;
}

// Load balancing and priority system
void AISystemIntegrator::SetTaskPriority(const std::string& taskId, AITaskPriority priority) {
    for (auto& task : m_TaskQueue) {
        if (task.id == taskId) {
            task.priority = priority;
            return;
        }
    }
}

bool AISystemIntegrator::QueueAITask(const std::string& taskId, std::function<void()> task, AITaskPriority priority) {
    if (!task) return false;
    
    m_TaskQueue.push_back({
        taskId,
        task,
        priority,
        std::chrono::high_resolution_clock::now()
    });
    
    m_Metrics.totalTasksQueued++;
    return true;
}

void AISystemIntegrator::ProcessTaskQueue() {
    if (m_TaskQueue.empty()) return;
    
    // Sort tasks by priority
    std::sort(m_TaskQueue.begin(), m_TaskQueue.end(), 
        [](const AITask& a, const AITask& b) {
            return static_cast<int>(a.priority) > static_cast<int>(b.priority);
        });
    
    // Process tasks
    auto now = std::chrono::high_resolution_clock::now();
    while (!m_TaskQueue.empty()) {
        auto& task = m_TaskQueue.front();
        
        // Skip if system is overloaded (except for Critical tasks)
        if (m_SystemHealth.isOverloaded && task.priority != AITaskPriority::Critical) {
            break;
        }
        
        task.task();
        m_Metrics.totalTasksProcessed++;
        m_TaskQueue.erase(m_TaskQueue.begin());
    }
}

// System health monitoring
AISystemHealth AISystemIntegrator::GetSystemHealth() const {
    return m_SystemHealth;
}

void AISystemIntegrator::MonitorSystemHealth() {
    auto now = std::chrono::high_resolution_clock::now();
    auto timeSinceLastCheck = std::chrono::duration<double>(now - m_LastHealthCheck).count();
    
    // Check health every second
    if (timeSinceLastCheck < 1.0) return;
    
    m_LastHealthCheck = now;
    
    // Check CPU and memory usage
    double cpuUsage = CalculateCPUUsage();
    double memoryUsage = CalculateMemoryUsage();
    
    m_SystemHealth.isOverloaded = cpuUsage > m_MaxCpuUsage || memoryUsage > m_MaxMemoryUsage;
    m_SystemHealth.needsThrottling = m_SystemHealth.isOverloaded;
    
    // Calculate system stability (0.0 to 1.0)
    m_SystemHealth.systemStability = 1.0 - (
        (cpuUsage / m_MaxCpuUsage + memoryUsage / m_MaxMemoryUsage) / 2.0
    );
    
    // Clear old warnings
    m_SystemHealth.activeWarnings.clear();
    m_SystemHealth.recoveryActions.clear();
    
    // Add warnings based on metrics
    if (cpuUsage > m_MaxCpuUsage) {
        AddHealthWarning("High CPU usage detected");
        m_SystemHealth.recoveryActions.push_back("Reduce AI agent update frequency");
    }
    
    if (memoryUsage > m_MaxMemoryUsage) {
        AddHealthWarning("High memory usage detected");
        m_SystemHealth.recoveryActions.push_back("Clear non-critical AI caches");
    }
    
    if (m_Metrics.averageResponseTime > 0.1) {
        AddHealthWarning("High response time detected");
        m_SystemHealth.recoveryActions.push_back("Prioritize critical AI tasks");
    }
}

bool AISystemIntegrator::AttemptSystemRecovery() {
    if (!m_SystemHealth.isOverloaded) return true;
    
    // Implement recovery actions
    for (const auto& action : m_SystemHealth.recoveryActions) {
        if (action == "Reduce AI agent update frequency") {
            // Reduce update frequency for non-critical agents
            m_AIController->SetUpdateFrequency(0.5f);
        }
        else if (action == "Clear non-critical AI caches") {
            // Clear caches
            m_PathfindingSystem->ClearCache();
            m_DecisionSystem->ClearCache();
        }
        else if (action == "Prioritize critical AI tasks") {
            // Remove non-critical tasks from queue
            m_TaskQueue.erase(
                std::remove_if(m_TaskQueue.begin(), m_TaskQueue.end(),
                    [](const AITask& task) {
                        return task.priority == AITaskPriority::Low || 
                               task.priority == AITaskPriority::Background;
                    }
                ),
                m_TaskQueue.end()
            );
        }
    }
    
    return true;
}

void AISystemIntegrator::AddHealthWarning(const std::string& warning) {
    m_SystemHealth.activeWarnings.push_back(warning);
    std::cerr << "AI System Health Warning: " << warning << std::endl;
}

// Performance optimization
void AISystemIntegrator::EnableDynamicThrottling(bool enable) {
    m_DynamicThrottlingEnabled = enable;
}

void AISystemIntegrator::SetPerformanceTargets(double maxCpuUsage, double maxMemoryUsage) {
    m_MaxCpuUsage = maxCpuUsage;
    m_MaxMemoryUsage = maxMemoryUsage;
}

void AISystemIntegrator::AdjustWorkload() {
    if (!m_DynamicThrottlingEnabled) return;
    
    if (m_SystemHealth.needsThrottling) {
        // Reduce workload
        size_t toRemove = m_TaskQueue.size() / 4; // Remove 25% of tasks
        while (toRemove > 0 && !m_TaskQueue.empty()) {
            auto it = std::find_if(m_TaskQueue.begin(), m_TaskQueue.end(),
                [](const AITask& task) {
                    return task.priority == AITaskPriority::Low;
                });
            if (it != m_TaskQueue.end()) {
                m_TaskQueue.erase(it);
                toRemove--;
            } else {
                break;
            }
        }
    }
}

// Debug visualization
void AISystemIntegrator::SetDebugVisualization(const AIDebugVisualization& settings) {
    m_DebugSettings = settings;
    m_DebugEnabled = true;
}

void AISystemIntegrator::UpdateDebugDisplay() {
    if (!m_DebugEnabled) return;
    
    if (m_DebugSettings.showPathfinding) {
        m_PathfindingSystem->DrawDebugInfo(m_DebugSettings.debugColor, m_DebugSettings.debugScale);
    }
    
    if (m_DebugSettings.showDecisionTrees) {
        m_DecisionSystem->DrawDebugInfo(m_DebugSettings.debugColor, m_DebugSettings.debugScale);
    }
    
    if (m_DebugSettings.showBehaviorStates) {
        m_AIController->DrawDebugInfo(m_DebugSettings.debugColor, m_DebugSettings.debugScale);
    }
    
    if (m_DebugSettings.showPerformanceMetrics) {
        DrawAIDebugInfo();
    }
}

void AISystemIntegrator::DrawAIDebugInfo() {
    std::stringstream ss;
    ss << "AI System Metrics:\n"
       << "CPU Usage: " << m_Metrics.cpuUsage << "%\n"
       << "Memory Usage: " << m_Metrics.memoryUsage << "%\n"
       << "Active Agents: " << m_Metrics.activeAgents << "\n"
       << "Tasks Queued: " << m_Metrics.totalTasksQueued << "\n"
       << "Tasks Processed: " << m_Metrics.totalTasksProcessed << "\n"
       << "Avg Response Time: " << m_Metrics.averageResponseTime << "ms\n"
       << "System Stability: " << m_SystemHealth.systemStability * 100 << "%";
    
    // Render debug text (implementation depends on rendering system)
    std::cout << ss.str() << std::endl;
}

// Helper methods for CPU and memory usage calculation
double AISystemIntegrator::CalculateCPUUsage() {
    // This is a simplified implementation
    // In a real system, you would use platform-specific APIs
    double totalTime = m_Metrics.pathfindingTime + 
                      m_Metrics.obstacleAvoidanceTime +
                      m_Metrics.decisionMakingTime +
                      m_Metrics.combatUpdateTime;
    
    return (totalTime / 0.016667) * 100.0; // Assuming 60 FPS target
}

double AISystemIntegrator::CalculateMemoryUsage() {
    // This is a simplified implementation
    // In a real system, you would use platform-specific APIs
    size_t totalMemory = sizeof(AISystemIntegrator) +
                        m_TaskQueue.capacity() * sizeof(AITask) +
                        m_SubsystemStatus.size() * sizeof(std::pair<std::string, AISubsystemStatus>) +
                        m_LastErrors.size() * sizeof(std::pair<std::string, std::string>);
    
    // Convert to percentage (assuming 1GB max memory)
    return (static_cast<double>(totalMemory) / (1024 * 1024 * 1024)) * 100.0;
}

void AISystemIntegrator::Shutdown() {
    // Shutdown all subsystems in reverse order with status tracking
    SetSubsystemStatus("ObstacleAvoidance", AISubsystemStatus::ShuttingDown);
    if (m_ObstacleAvoidance) m_ObstacleAvoidance->Shutdown();
    
    SetSubsystemStatus("CombatManager", AISubsystemStatus::ShuttingDown);
    if (m_CombatManager) m_CombatManager->Shutdown();
    
    SetSubsystemStatus("DecisionSystem", AISubsystemStatus::ShuttingDown);
    if (m_DecisionSystem) m_DecisionSystem->Shutdown();
    
    SetSubsystemStatus("AIController", AISubsystemStatus::ShuttingDown);
    if (m_AIController) m_AIController->Shutdown();
    
    SetSubsystemStatus("PathfindingSystem", AISubsystemStatus::ShuttingDown);
    if (m_PathfindingSystem) m_PathfindingSystem->Shutdown();
    
    // Reset all systems
    m_ObstacleAvoidance.reset();
    m_CombatManager.reset();
    m_DecisionSystem.reset();
    m_AIController.reset();
    m_PathfindingSystem.reset();
    
    // Clear status and metrics
    m_SubsystemStatus.clear();
    m_LastErrors.clear();
    m_TaskQueue.clear();
    m_SystemHealth = {false, false, {}, {}, 1.0};
    m_Metrics = {0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0.0, 0.0, 0, 0, 0.0};
    
    std::cout << "AI System Integration shutdown completed" << std::endl;
}
