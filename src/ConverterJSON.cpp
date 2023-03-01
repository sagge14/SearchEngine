//
// Created by user on 04.02.2023.
//

#include "ConverterJSON.h"

void ConverterJSON::setSettings(const Settings &val) {

    nh::json jsonSettings;

    jsonSettings["config"]["Name"] = val.name;
    jsonSettings["config"]["Version"] = val.version;
    jsonSettings["config"]["max_response"] = val.maxResponse;
    jsonSettings["config"]["thread_count"] = val.threadCount;
    jsonSettings["config"]["dir"] = val.dir;
    jsonSettings["config"]["ind_time"] = val.indTime;
    jsonSettings["config"]["search_time"] = val.searchTime;
    jsonSettings["config"]["text_request"] = val.requestText;
    jsonSettings["config"]["exact_search"] = val.exactSearch;
    jsonSettings["Files"] = val.files;

    std::ofstream jsonFileSettings("Films.json");
    jsonFileSettings << jsonSettings;
    jsonFileSettings.close();

}

Settings ConverterJSON::getSettings(const std::string& jsonPath) {

    Settings s;
    nh::json jsonSettings;
    std::ifstream jsonFileSettings(jsonPath);

    try
    {
        jsonSettings = nh::json::parse(jsonFileSettings);
        jsonFileSettings.close();

        jsonSettings.at("config").at("Name").get_to(s.name);
        jsonSettings.at("config").at("Version").get_to(s.version);
        jsonSettings.at("config").at("max_response").get_to(s.maxResponse);
        jsonSettings.at("config").at("thread_count").get_to(s.threadCount);
        jsonSettings.at("config").at("dir").get_to(s.dir);
        jsonSettings.at("config").at("exact_search").get_to(s.exactSearch);
        jsonSettings.at("config").at("ind_time").get_to(s.indTime);
        jsonSettings.at("config").at("search_time").get_to(s.searchTime);
        jsonSettings.at("config").at("text_request").get_to(s.requestText);
        jsonSettings.at("Files").get_to(s.files);
    }
    catch (nh::json::parse_error& ex)
    {
        std::cerr << "JSON parse error at byte " << ex.byte << std::endl;
        throw myExp(jsonPath);
    }
    catch (...)
    {
        throw myExp(jsonPath);
    }

    return s;
}

std::vector<std::string> ConverterJSON::getRequests(const std::string& jsonPath) {

    std::vector<std::string> requests;
    nh::json jsonRequests;
    std::ifstream jsonFileRequests(jsonPath);

    try
    {
        jsonRequests = nh::json::parse(jsonFileRequests);
        jsonFileRequests.close();
        jsonRequests.at("Requests").get_to(requests);
    }
    catch (nh::json::parse_error& ex)
    {
        return std::move(requests);
    }
    catch (...)
    {
        return std::move(requests);
    }

    return requests;
}

void ConverterJSON::putAnswers(const listAnswers& answers, const std::string& jsonPath) {

    nh::json jsonAnswers;

    for(const auto& a: answers)
    {
        if(!std::get<0>(a).empty())
        {
            jsonAnswers["Answers"][std::get<1>(a)]["result"] = true;
            jsonAnswers["Answers"][std::get<1>(a)]["relevance"] = std::get<0>(a);
        }
        else
            jsonAnswers["Answers"][std::get<1>(a)]["result"] = false;
    }

    std::ofstream jsonFileSettings("Answers.json");
    jsonFileSettings << jsonAnswers;
    jsonFileSettings.close();
}


