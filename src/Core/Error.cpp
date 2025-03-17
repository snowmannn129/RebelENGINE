#include "core/Error.h"
#include <sstream>

namespace rebel::core {

Error::Error(ErrorCode code, 
             const std::string& message,
             const std::string& file,
             int line)
    : errorCode(code)
    , errorMessage(message)
    , errorFile(file)
    , errorLine(line) {
    
    // Log the error when it's created
    std::stringstream ss;
    ss << "Error [" << static_cast<int>(code) << "]: " << message;
    
    // Log using our logging system
    Logger::getInstance().error(ss.str());
}

void Error::throwError(ErrorCode code,
                      const std::string& message,
                      const std::string& file,
                      int line) {
    throw Error(code, message, file, line);
}

} // namespace rebel::core
