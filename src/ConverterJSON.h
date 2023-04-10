//
// Created by user on 04.02.2023.
//
#pragma once
#include <fstream>
#include <utility>
#include <vector>
#include <list>
#include <iostream>
#include "nlohmann/json.hpp"
#include "Settings.h"
#include <exception>

namespace nh = nlohmann;
typedef std::list<std::pair<std::string, float>> listAnswer;
typedef std::list<std::pair<listAnswer, std::string>> listAnswers;

class ConverterJSON {

public:
    /** @param myExp исключения выбрасываемые при проблемах чтения информации из json файлов.
        @param setSettings функция записи настроек сервера в json файл - ни где пока не используется, но может пригодиться..
        @param getSettings функция получения настроек сервера из json файла.
        @param getRequests функция получения запросов из json файла, адресованных поисковому серверу.
        @param putAnswers функция записи ответов сервера на запросы в json файл. */

    struct myExp : public std::exception
    {
    public:
        std::string file;
        myExp() = default;
        explicit myExp(std::string  _file): file(std::move(_file)){};
        void show () const { std::cout << "Error pars settings, check file '" << file <<"'"  << std::endl;}
    };

    ConverterJSON() = default;
    static void setSettings(const Settings& val);
    static Settings getSettings(const std::string& jsonPath = "Settings.json");
    static std::vector<std::string> getRequests(const std::string& jsonPath = "Requests.json");
    static std::vector<std::string> getRequestsFromString(const std::string& jsonString);
    static std::string putAnswers(const listAnswers& answers, const std::string& jsonPath = "Answers.json");
};

