//
// Created by user on 04.02.2023.
//

#include "ConverterJSON.h"

void ConverterJSON::setSettings(const Settings &val) {

    nh::json jsonSettings;

    jsonSettings["Name"] = val.name;
    jsonSettings["Version"] = val.version;
    jsonSettings["max_response"] = val.maxResponse;
    jsonSettings["thread_count"] = val.threadCount;
    jsonSettings["dir"] = val.dir;
    jsonSettings["ind_time"] = val.indTime;
    jsonSettings["search_time"] = val.searchTime;
    jsonSettings["Files"] = val.files;
    jsonSettings["text_request"] = val.requestText;

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
    }
    catch (nh::json::parse_error& ex)
    {
        jsonFileSettings.close();
        std::cerr << "JSON parse error at byte " << ex.byte << std::endl;
        throw myExp();
    }


    jsonSettings.at("Name").get_to(s.name);
    jsonSettings.at("Version").get_to(s.version);
    jsonSettings.at("Files").get_to(s.files);
    jsonSettings.at("max_response").get_to(s.maxResponse);
    jsonSettings.at("thread_count").get_to(s.threadCount);
    jsonSettings.at("dir").get_to(s.dir);
    jsonSettings.at("ind_time").get_to(s.indTime);
    jsonSettings.at("search_time").get_to(s.searchTime);
    jsonSettings.at("text_request").get_to(s.requestText);

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
    }
    catch (nh::json::parse_error& ex)
    {
        jsonFileRequests.close();
        return std::move(requests);
    }
    catch (...)
    {
        jsonFileRequests.close();
        return std::move(requests);
    }

    jsonRequests.at("Requests").get_to(requests);

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


