#pragma once

#include <memory>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace RebelCAD {
namespace Modeling {

class Mesh;

class PreviewCache {
public:
    PreviewCache();
    ~PreviewCache();

    // Update cache with new preview mesh
    void update(std::shared_ptr<Mesh> previewMesh);

    // Get cached preview mesh
    std::shared_ptr<Mesh> get() const;

    // Clear cache
    void clear();

    // Check if cache is valid
    bool isValid() const;

private:
    // Cache data
    std::shared_ptr<Mesh> m_cachedMesh;
    std::chrono::steady_clock::time_point m_lastUpdateTime;
    bool m_isValid;
    mutable std::mutex m_mutex;

    // Cache settings
    static constexpr auto CACHE_TIMEOUT = std::chrono::seconds(5);

    // Validate cache timestamp
    bool validateTimestamp() const;
};

} // namespace Modeling
} // namespace RebelCAD
