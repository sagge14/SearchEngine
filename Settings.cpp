//
// Created by user on 08.02.2023.
//

#include "Settings.h"

Settings &Settings::operator=(Settings &&s) noexcept
{
    std::swap(name,s.name);
    std::swap(version,s.version);
    std::swap(maxResponse,s.maxResponse);
    std::swap(files,s.files);
    std::swap(dir,s.dir);
    std::swap(threadCount,s.threadCount);
    std::swap(indTime,s.indTime);
    std::swap(requestText,s.requestText);
    s.name = "";
    s.version = "";
    s.files.clear();
    s.maxResponse = 0;
    s.dir = "";
    s.threadCount = 0;
    s.indTime = 0;
    s.requestText = false;
    return *this;
}

Settings::Settings(Settings &&s) noexcept : Settings()
{
    std::swap(name,s.name);
    std::swap(version,s.version);
    std::swap(maxResponse,s.maxResponse);
    std::swap(files,s.files);
    std::swap(dir,s.dir);
    std::swap(threadCount,s.threadCount);
    std::swap(indTime,s.indTime);
    std::swap(requestText,s.requestText);
}
