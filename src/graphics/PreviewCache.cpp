#include "modeling/PreviewCache.hpp"
#include "core/GarbageCollector.hpp"
#include <stdexcept>

namespace RebelCAD {
namespace Modeling {

PreviewCache::PreviewCache()
    : m_isValid(false)
{
}

PreviewCache::~PreviewCache() {
    clear();
}

void PreviewCache::update(std::shared_ptr<Mesh> previewMesh) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!previewMesh) {
        throw std::invalid_argument("Cannot cache null preview mesh");
    }

    m_cachedMesh = previewMesh;
    m_lastUpdateTime = std::chrono::steady_clock::now();
    m_isValid = true;

    // Register with garbage collector
    GarbageCollector::getInstance().registerObject(m_cachedMesh);
}

std::shared_ptr<Mesh> PreviewCache::get() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!isValid()) {
        return nullptr;
    }

    // Mark as recently used in garbage collector
    GarbageCollector::getInstance().markUsed(m_cachedMesh.get());
    return m_cachedMesh;
}

void PreviewCache::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_cachedMesh) {
        GarbageCollector::getInstance().unregisterObject(m_cachedMesh);
        m_cachedMesh.reset();
    }
    m_isValid = false;
}

bool PreviewCache::isValid() const {
    return m_isValid && validateTimestamp();
}

bool PreviewCache::validateTimestamp() const {
    if (!m_isValid || !m_cachedMesh) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    return (now - m_lastUpdateTime) <= CACHE_TIMEOUT;
}

} // namespace Modeling
} // namespace RebelCAD
