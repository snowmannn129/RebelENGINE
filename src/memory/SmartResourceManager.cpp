#include "core/SmartResourceManager.h"

namespace RebelCAD {
namespace Core {

SmartResourceManager& SmartResourceManager::getInstance() {
    static SmartResourceManager instance;
    return instance;
}

} // namespace Core
} // namespace RebelCAD
