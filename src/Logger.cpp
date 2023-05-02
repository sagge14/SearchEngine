//
// Created by user on 02.05.2023.
//

#include "Logger.h"


 void Logger::addToLog(const std::string& _s) {
    static Logger instance;

    char dataTime[20];
    time_t now = time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    std::lock_guard<std::mutex> myLock(instance.logMutex);

    instance.logFile.open("log.log", std::ios::app);
    instance.logFile << "[" << dataTime << "] " << _s << std::endl;
    instance.logFile.close();
}
