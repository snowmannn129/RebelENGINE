#include "graphics/ViewSyncManager.h"
#include <algorithm>
#include <mutex>

namespace RebelCAD {
namespace Graphics {

// Initialize static memory pool
Core::MemoryPool<ViewSyncGroup> ViewSyncManager::s_groupPool;

// ViewSyncGroup Implementation
//---------------------------

ViewSyncGroup::ViewSyncGroup(const std::string& name)
    : m_name(name)
    , m_updating(false)
    , m_currentState() {
}

ViewSyncGroup::~ViewSyncGroup() = default;

bool ViewSyncGroup::addViewport(Viewport* viewport) {
    if (!viewport) return false;
    
    // Check if viewport already exists in group
    if (containsViewport(viewport)) {
        return false;
    }
    
    m_viewports.push_back(viewport);
    
    // Apply current state to new viewport
    viewport->setView(
        m_currentState.position,
        m_currentState.target,
        m_currentState.upVector
    );
    
    return true;
}

void ViewSyncGroup::removeViewport(Viewport* viewport) {
    auto it = std::find(m_viewports.begin(), m_viewports.end(), viewport);
    if (it != m_viewports.end()) {
        m_viewports.erase(it);
    }
}

bool ViewSyncGroup::containsViewport(Viewport* viewport) const {
    return std::find(m_viewports.begin(), m_viewports.end(), viewport) != m_viewports.end();
}

ViewState ViewSyncGroup::getViewState() const {
    return m_currentState;
}

void ViewSyncGroup::updateViewState(const ViewState& state) {
    if (m_updating.exchange(true)) {
        return; // Already updating
    }
    
    m_currentState = state;
    
    // Update all viewports in group
    for (auto viewport : m_viewports) {
        viewport->setView(
            state.position,
            state.target,
            state.upVector
        );
        // TODO: Update projection and other view parameters
    }
    
    m_updating = false;
}

// ViewSyncManager Implementation
//-----------------------------

ViewSyncManager::ViewSyncManager()
    : m_updating(false) {
}

ViewSyncManager::~ViewSyncManager() = default;

ViewSyncGroup* ViewSyncManager::createGroup(const std::string& name) {
    if (!tryLock()) {
        return nullptr;
    }
    
    // Check if group already exists
    if (m_groups.find(name) != m_groups.end()) {
        unlock();
        return nullptr;
    }
    
    // Create new group
    auto group = std::make_unique<ViewSyncGroup>(name);
    ViewSyncGroup* ptr = group.get();
    m_groups[name] = std::move(group);
    
    unlock();
    return ptr;
}

void ViewSyncManager::removeGroup(const std::string& name) {
    if (!tryLock()) {
        return;
    }
    
    m_groups.erase(name);
    
    unlock();
}

ViewSyncGroup* ViewSyncManager::getGroup(const std::string& name) {
    if (!tryLock()) {
        return nullptr;
    }
    
    auto it = m_groups.find(name);
    ViewSyncGroup* group = it != m_groups.end() ? it->second.get() : nullptr;
    
    unlock();
    return group;
}

bool ViewSyncManager::addToGroup(const std::string& groupName, Viewport* viewport) {
    if (!tryLock()) {
        return false;
    }
    
    auto it = m_groups.find(groupName);
    bool success = false;
    
    if (it != m_groups.end()) {
        success = it->second->addViewport(viewport);
    }
    
    unlock();
    return success;
}

void ViewSyncManager::removeFromGroup(const std::string& groupName, Viewport* viewport) {
    if (!tryLock()) {
        return;
    }
    
    auto it = m_groups.find(groupName);
    if (it != m_groups.end()) {
        it->second->removeViewport(viewport);
    }
    
    unlock();
}

void ViewSyncManager::updateGroupState(const std::string& groupName, const ViewState& state) {
    if (!tryLock()) {
        return;
    }
    
    auto it = m_groups.find(groupName);
    if (it != m_groups.end()) {
        it->second->updateViewState(state);
    }
    
    unlock();
}

std::vector<std::string> ViewSyncManager::getViewportGroups(Viewport* viewport) const {
    std::vector<std::string> groups;
    
    if (!tryLock()) {
        return groups;
    }
    
    for (const auto& [name, group] : m_groups) {
        if (group->containsViewport(viewport)) {
            groups.push_back(name);
        }
    }
    
    unlock();
    return groups;
}

void ViewSyncManager::clear() {
    if (!tryLock()) {
        return;
    }
    
    m_groups.clear();
    
    unlock();
}

bool ViewSyncManager::tryLock() const {
    bool expected = false;
    return m_updating.compare_exchange_strong(expected, true);
}

void ViewSyncManager::unlock() const {
    m_updating = false;
}

} // namespace Graphics
} // namespace RebelCAD
