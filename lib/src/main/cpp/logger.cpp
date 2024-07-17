#include "logger.h"
#include "string"
#include "fstream"
#include "sstream"

// -- LOGGER
void logger(const char *outDir) {
    std::stringstream logMessage;
    logMessage << "Script Started";
    // logToFile = logMessage.str();
    auto outLogPath = std::string(outDir).append("/files/zhacks_log.txt");
    std::ofstream outStream(outLogPath);
    outStream << logMessage.str();
    outStream.close();
}
// -- END LOGGER