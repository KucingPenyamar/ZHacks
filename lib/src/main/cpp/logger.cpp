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

#define gamePackage "com.unity.test"

void flog(const char* logMessage) { 
    std::string appData = "/data/data/";
    std::string pckg = gamePackage;
    std::string outPath = appData + pckg;
    std::string logOutPath = outPath + "/files/log.txt"; 
    std::ofstream outStream(logOutPath, std::ios::app); 
    outStream << logMessage << std::endl; 
    outStream.close();
}
// -- END LOGGER