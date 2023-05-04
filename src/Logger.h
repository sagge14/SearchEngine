//
// Created by user on 02.05.2023.
//

#ifndef SEARCHENGINE_TEST_LOGGER_H
#define SEARCHENGINE_TEST_LOGGER_H


#include <string>
#include <mutex>
#include <fstream>

class Logger {

    std::mutex logMutex;
    std::ofstream logFile;

    Logger() = default;

public:

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger& operator=(const Logger &) = delete;
    Logger& operator=(Logger &&) = delete;

    static void addToLog(const std::string& s);
};


#endif //SEARCHENGINE_TEST_LOGGER_H
