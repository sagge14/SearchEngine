//
// Created by user on 08.02.2023.
//

#include "Settings.h"


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
    std::cout << "Show request as text:\t\t" << std::boolalpha << requestText << std::endl;
    std::cout << "Use exact search:\t\t" << std::boolalpha << exactSearch << std::endl << std::endl;
}

Settings::Settings() {

    name = "TestServer";
    version = "1.1";
    dir = "";
    threadCount = 1;
    maxResponse = 5;
    exactSearch = false;

}

Settings *Settings::getSettings() {

    if(!settings)
        settings = new Settings();

    return settings;

}
