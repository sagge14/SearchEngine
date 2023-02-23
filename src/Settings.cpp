//
// Created by user on 08.02.2023.
//

#include "Settings.h"

Settings& Settings::operator=(Settings &&s) noexcept
{
    std::swap(name,s.name);
    std::swap(version,s.version);
    std::swap(maxResponse,s.maxResponse);
    std::swap(files,s.files);
    std::swap(dir,s.dir);
    std::swap(threadCount,s.threadCount);
    std::swap(searchTime,s.searchTime);
    std::swap(indTime,s.indTime);
    std::swap(requestText,s.requestText);
    s.name = "";
    s.version = "";
    s.files.clear();
    s.maxResponse = 0;
    s.dir = "";
    s.threadCount = 0;
    s.indTime = 0;
    s.searchTime = 0;
    s.requestText = false;
    return *this;
}

Settings::Settings(Settings &&s) noexcept: Settings()
{
    std::swap(name,s.name);
    std::swap(version,s.version);
    std::swap(maxResponse,s.maxResponse);
    std::swap(files,s.files);
    std::swap(dir,s.dir);
    std::swap(threadCount,s.threadCount);
    std::swap(indTime,s.indTime);
    std::swap(requestText,s.requestText);
    std::swap(searchTime,s.searchTime);
}

void Settings::show() const
{
    std::cout << "--- Server information ---" << std::endl;
    std::cout << std::endl << "Name:\t\t\t\t" << name << std::endl;
    std::cout << "Version:\t\t\t" << version << std::endl;
    std::cout << "Number of maximum responses:\t" << maxResponse << std::endl;
    if(!dir.empty())
        std::cout << "The directory for indexing:\t" << dir << std::endl;
    std::cout << "Thread count:\t\t\t";
    if(threadCount)
        std::cout << threadCount << std::endl;
    else
        std::cout << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Index database update period:\t" << indTime << " seconds" << std::endl;
    std::cout << "Index database update period:\t" << searchTime << " milliseconds" << std::endl;
    std::cout << "Show request as text:\t\t" << std::boolalpha << requestText << std::endl << std::endl;
}
