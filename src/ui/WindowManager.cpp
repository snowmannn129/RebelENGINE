#include "ui/WindowManager.h"
#include "core/Log.h"
#include "imgui.h"
#include <algorithm>

namespace RebelCAD {
namespace UI {

WindowManager& WindowManager::getInstance() {
    static WindowManager instance;
    return instance;
}

WindowManager::~WindowManager() {
    destroyAllWindows();
}

std::shared_ptr<MainWindow> WindowManager::createWindow(const std::string& title, int width, int height) {
    auto window = std::make_shared<MainWindow>(title, width, height);
    try {
        window->initialize();
        m_Windows.push_back(window);
        
        // Set as active window if it's the first window
        if (m_Windows.size() == 1) {
            setActiveWindow(window);
        }
        
        LOG_INFO("Created new window: {}", title);
        return window;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Failed to create window: {}", e.what());
        return nullptr;
    }
}

void WindowManager::destroyWindow(std::shared_ptr<MainWindow> window) {
    if (!window) return;

    auto it = std::find(m_Windows.begin(), m_Windows.end(), window);
    if (it != m_Windows.end()) {
        // If destroying active window, set another window as active
        if (window == m_ActiveWindow && m_Windows.size() > 1) {
            setActiveWindow((window == m_Windows.front()) ? m_Windows[1] : m_Windows.front());
        }
        else if (m_Windows.size() == 1) {
            m_ActiveWindow.reset();
        }

        // Remove the window from the list
        m_Windows.erase(it);
        LOG_INFO("Destroyed window: {}", window->getTitle());
    }
}

void WindowManager::destroyAllWindows() {
    m_Windows.clear();
    m_ActiveWindow.reset();
    LOG_INFO("All windows destroyed");
}

std::vector<std::shared_ptr<MainWindow>> WindowManager::getWindows() const {
    return m_Windows;
}

std::shared_ptr<MainWindow> WindowManager::getActiveWindow() const {
    return m_ActiveWindow;
}

size_t WindowManager::getWindowCount() const {
    return m_Windows.size();
}

void WindowManager::updateAll() {
    removeClosedWindows();
    
    for (auto& window : m_Windows) {
        if (window && !window->shouldClose()) {
            // Handle window focus/activation
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) && 
                window != m_ActiveWindow) {
                setActiveWindow(window);
            }
        }
    }
}

void WindowManager::renderAll() {
    for (auto& window : m_Windows) {
        if (window && !window->shouldClose()) {
            window->render();
        }
    }
}

void WindowManager::setActiveWindow(std::shared_ptr<MainWindow> window) {
    if (window && window != m_ActiveWindow) {
        m_ActiveWindow = window;
        LOG_INFO("Active window changed to: {}", window->getTitle());
    }
}

void WindowManager::removeClosedWindows() {
    m_Windows.erase(
        std::remove_if(
            m_Windows.begin(),
            m_Windows.end(),
            [](const auto& window) {
                return !window || window->shouldClose();
            }
        ),
        m_Windows.end()
    );
}

} // namespace UI
} // namespace RebelCAD
