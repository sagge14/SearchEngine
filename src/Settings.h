//
// Created by user on 08.02.2023.
//

#pragma once
#include <iostream>
#include <vector>
#include <thread>

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

    Settings() = default;
    Settings(const Settings& s) = default;

    Settings(Settings&& s) noexcept ;
};

