#include "logger.h"
#include "string"
#include "fstream"
#include "sstream"

// -- LOGGER
void logger(const char *outDir) {
    std::stringstream logMessage;
    logMessage << "Script Started";
    // logToFile = logMessage.str();
    auto outLogPath = std::string(outDir).append("/files/log.txt");
    std::ofstream outStream(outLogPath);
    outStream << logMessage.str();
    outStream.close();
}
void flog(const char* logMessage) { // Change to const char*
    std::string logOutPath = std::string(gameDataDir).append("/files/log.txt"); // Add semicolon
    std::ofstream outStream(logOutPath, std::ios::app); // Append to the file
    outStream << logMessage << std::endl; // Add a newline at the end of each log message
    outStream.close();
}
// -- END LOGGER