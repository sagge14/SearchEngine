//
// Created by user on 04.02.2023.
//
#pragma once
#include <fstream>
#include <vector>
#include <list>
#include <tuple>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Settings.h"
#include <exception>

namespace nh = nlohmann;
typedef std::list<std::tuple<std::string, float>> listAnswer;
typedef std::list<std::tuple<listAnswer, std::string>> listAnswers;

class ConverterJSON {

public:

    struct myExp : public std::exception
    {
    public:
        myExp() = default;
        static void show () { std::cout << "error pars settings" << std::endl;}
    };

    ConverterJSON() = default;
    static void setSettings(const Settings& val);
    static Settings getSettings(const std::string& jsonPath = "Settings.json");
    static std::vector<std::string> getRequests(const std::string& jsonPath = "Requests.json");
    static void putAnswers(const listAnswers& answers, const std::string& jsonPath = "Answers.json");
};

