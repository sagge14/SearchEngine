//
// Created by user on 08.02.2023.
//

#pragma once
#include <iostream>
#include <vector>
#include <thread>

#define TEST_MODE true

struct Settings
{
    std::string name;
    std::string version;
    std::string dir;
    int threadCount{};
    size_t indTime{};
    size_t searchTime{};
    int maxResponse{};
    bool requestText{};
    std::vector<std::string> files;

    void show() const;

    Settings& operator=(Settings&& s) noexcept ;
    Settings& operator=(const Settings& s) = default;

    Settings()
    {
        name = "TestServer";
        version = "1.1";
        dir = "";
        threadCount = 1;
        maxResponse = 5;
    }
    Settings(const Settings& s) = default;

    Settings(Settings&& s) noexcept ;
};

