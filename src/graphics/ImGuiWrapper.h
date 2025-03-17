#pragma once

// Include ImGui headers in the correct order
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// ImGui docking flags and enums
#ifndef ImGuiDockNodeFlags_None
#define ImGuiDockNodeFlags_None              0
#define ImGuiDockNodeFlags_DockSpace         1 << 10
#endif

#ifndef ImGuiWindowFlags_NoDocking
#define ImGuiWindowFlags_NoDocking           1 << 24
#endif

namespace RebelCAD {
namespace UI {

/**
 * @brief Singleton wrapper for ImGui context and initialization
 */
class ImGuiWrapper {
public:
    static ImGuiWrapper& getInstance() {
        static ImGuiWrapper instance;
        return instance;
    }

    // Delete copy constructor and assignment
    ImGuiWrapper(const ImGuiWrapper&) = delete;
    ImGuiWrapper& operator=(const ImGuiWrapper&) = delete;

    /**
     * @brief Initialize ImGui context and backends
     */
    void initialize() {
        if (m_Initialized) return;

        // Initialize GLFW
        if (!glfwInit()) return;

        // Create window for ImGui context
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window for testing
        m_Window = glfwCreateWindow(1280, 720, "RebelCAD", nullptr, nullptr);
        if (!m_Window) {
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(m_Window);

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        m_Initialized = true;
    }

    /**
     * @brief Shutdown ImGui context and cleanup
     */
    void shutdown() {
        if (!m_Initialized) return;

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
        glfwTerminate();

        m_Initialized = false;
    }

    /**
     * @brief Check if ImGui is initialized
     */
    bool isInitialized() const { return m_Initialized; }

    /**
     * @brief Get the GLFW window handle
     */
    GLFWwindow* getWindow() const { return m_Window; }

private:
    ImGuiWrapper() : m_Initialized(false), m_Window(nullptr) {}
    ~ImGuiWrapper() { shutdown(); }

    bool m_Initialized;
    GLFWwindow* m_Window;
};

// ImGui helper functions
inline void SetNextWindowDockID(ImGuiID id, ImGuiCond cond = 0) {
    ImGui::SetNextWindowDockID(id, cond);
}

inline ImGuiID DockSpace(const char* id, const ImVec2& size = ImVec2(0, 0), ImGuiDockNodeFlags flags = 0) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID(id);
    ImGui::DockSpace(dockspace_id, size, flags);
    ImGui::End();

    return dockspace_id;
}

inline ImGuiDockNode* DockBuilderGetNode(ImGuiID node_id) {
    return ImGui::DockBuilderGetNode(node_id);
}

inline void DockBuilderRemoveNode(ImGuiID node_id) {
    ImGui::DockBuilderRemoveNode(node_id);
}

inline ImGuiID DockBuilderAddNode(ImGuiID node_id = 0, ImGuiDockNodeFlags flags = 0) {
    return ImGui::DockBuilderAddNode(node_id, flags);
}

inline void DockBuilderSetNodeSize(ImGuiID node_id, ImVec2 size) {
    ImGui::DockBuilderSetNodeSize(node_id, size);
}

inline void DockBuilderFinish(ImGuiID node_id) {
    ImGui::DockBuilderFinish(node_id);
}

inline ImGuiID DockBuilderSplitNode(ImGuiID node_id, ImGuiDir split_dir, float size_ratio_for_node_at_dir, ImGuiID* out_id_at_dir, ImGuiID* out_id_at_opposite_dir) {
    return ImGui::DockBuilderSplitNode(node_id, split_dir, size_ratio_for_node_at_dir, out_id_at_dir, out_id_at_opposite_dir);
}

inline void DockBuilderDockWindow(const char* window_name, ImGuiID node_id) {
    ImGui::DockBuilderDockWindow(window_name, node_id);
}

} // namespace UI
} // namespace RebelCAD
