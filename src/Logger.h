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

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger& operator=(const Logger &) = delete;
    Logger& operator=(Logger &&) = delete;
    Logger() = default;
    ~Logger() = default;
public:

    static void addToLog(const std::string& s);
};


#endif //SEARCHENGINE_TEST_LOGGER_H
