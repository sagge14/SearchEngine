//
// Created by user on 04.02.2023.
//

#include "ConverterJSON.h"
#include "SearchServer.h"
#include "Logger.h"

void ConverterJSON::setSettings(const search_server::Settings &val) {
    /**
    Функция записи настроек сервера в json файл. */

    nh::json jsonSettings;

    jsonSettings["config"]["Name"] = val.name;
    jsonSettings["config"]["Version"] = val.version;
    jsonSettings["config"]["max_response"] = val.maxResponse;
    jsonSettings["config"]["asio_port"] = val.asioPort;
    jsonSettings["config"]["thread_count"] = val.threadCount;
    jsonSettings["config"]["dir"] = val.dir;
    jsonSettings["config"]["ind_time"] = val.indTime;
    jsonSettings["config"]["text_request"] = val.requestText;
    jsonSettings["config"]["exact_search"] = val.exactSearch;
    jsonSettings["Files"] = val.files;

    std::ofstream jsonFileSettings("Settings.json");
    jsonFileSettings << jsonSettings;
    jsonFileSettings.close();

}

void ConverterJSON::getSettings(const std::string& jsonPath) {
    /**
    Функция получения настроек сервера из json файла. */

    using namespace search_server;
    nh::json jsonSettings;
    std::ifstream jsonFileSettings(jsonPath);

    try
    {
        jsonSettings = nh::json::parse(jsonFileSettings);
        jsonFileSettings.close();

        jsonSettings.at("config").at("Name").get_to(Settings::getInstance().name);
        jsonSettings.at("config").at("Version").get_to(Settings::getInstance().version);
        jsonSettings.at("config").at("max_response").get_to(Settings::getInstance().maxResponse);
        jsonSettings.at("config").at("thread_count").get_to(Settings::getInstance().threadCount);
        jsonSettings.at("config").at("dir").get_to(Settings::getInstance().dir);
        jsonSettings.at("config").at("exact_search").get_to(Settings::getInstance().exactSearch);
        jsonSettings.at("config").at("ind_time").get_to(Settings::getInstance().indTime);
        jsonSettings.at("config").at("text_request").get_to(Settings::getInstance().requestText);
        jsonSettings.at("config").at("asio_port").get_to(Settings::getInstance().asioPort);
        jsonSettings.at("Files").get_to(Settings::getInstance().files);
    }
    catch (nh::json::parse_error& ex)
    {
        std::cerr << "JSON parse error (getSettings) at byte " << ex.byte << std::endl;
        Logger::addToLog("JSON parse error (getSettings) at byte " + to_string(ex.byte));
        throw myExp(jsonPath);
    }
}

std::vector<std::string> ConverterJSON::getRequests(const std::string& jsonPath) {
    /**
    функция получения запросов из json файла, адресованных поисковому серверу.
    Если json файла - некорректен или отсутствует возвращается пустой вектор,
    работа сервера не будет прекращена. */

    std::vector<std::string> requests;
    nh::json jsonRequests;
    std::ifstream jsonFileRequests(jsonPath);

    try
    {
        jsonRequests = nh::json::parse(jsonFileRequests);
        jsonFileRequests.close();
    }
    catch (nh::json::parse_error& ex)
    {
        std::cerr << "JSON parse error (getRequests) at byte " << ex.byte << std::endl;
        Logger::addToLog("JSON parse error (getRequests) at byte " + std::to_string(ex.byte));
        jsonFileRequests.close();
        return std::move(requests);
    }


    jsonRequests.at("Requests").get_to(requests);

    return requests;
}

std::string ConverterJSON::putAnswers(const listAnswers& answers, const std::string& jsonPath) {
    /**
     функция записи ответов сервера на запросы в json файл. */

    nh::json jsonAnswers;

    for(const auto& answer: answers)
    {
        if(!answer.first.empty())
        {
            jsonAnswers["Answers"][answer.second]["result"] = true;
            jsonAnswers["Answers"][answer.second]["relevance"] = answer.first;
        }
        else
            jsonAnswers["Answers"][answer.second]["result"] = false;
    }

    std::ofstream jsonFileSettings(jsonPath);

    jsonFileSettings << std::setw(2) << jsonAnswers;
    std::string ss = nh::to_string(jsonAnswers);
    jsonFileSettings.close();
    return std::move(ss);
}

std::vector<std::string> ConverterJSON::getRequestsFromString(const std::string &jsonString) {

    /**
     Тоже самое что для файла, только для строки. */

    std::vector<std::string> requests;
    nh::json jsonRequests;

    try
    {
        jsonRequests = nh::json::parse(jsonString);
    }
    catch (nh::json::parse_error& ex)
    {
        std::cerr << "JSON parse error (getRequestsFromString) at byte " << ex.byte << std::endl;
        Logger::addToLog("JSON parse error (getRequestsFromString) at byte " + std::to_string(ex.byte));
        return std::move(requests);
    }

    jsonRequests.at("Requests").get_to(requests);

    return requests;
}




