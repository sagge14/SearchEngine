//
// Created by user on 02.02.2023.
//
#pragma once
#include <iostream>
#include <list>
#include <set>
#include <map>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>
#include <tuple>
#include <utility>
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "Settings.h"

using namespace std;


//customStyle is "2016-05-18"

typedef string basicString;
typedef set<basicString> setWords;
typedef set<size_t> setFileInd;
typedef list<tuple<string, float>> listAnswer;
typedef list<tuple<listAnswer, string>> listAnswers;

class SearchServer {

private:

    enum class ErrorCodes
    {
        NAME,
        THREADCOUNT,
        WRONGDIRRECTORY,
        NOTFILESTOINDEX
    };
    struct Word {
        size_t count;
        basicString word;

        bool operator ==(const Word& other) const {return (word == other.word);}
        bool operator<(const Word& r) const {return count < r.count;}

        Word(basicString _word, size_t _count) : word(std::move(_word)), count(_count){};
    };
    struct FileResult
    {
        inline static size_t max{0};
        size_t sum{0};
        size_t fileInd{0};
        string filePath;

        [[nodiscard]] float getRelativeIndex() const { return (float) sum / (float) max;}

        bool operator == (const FileResult& other) const {return (sum == other.sum);}
        bool operator < (const FileResult& r) const {return sum > r.sum;}

        FileResult(size_t fileInd, const setWords& request, const InvertedIndex& index);
    };


    atomic<bool> work{};
    hash<string> hashRequest;
    InvertedIndex index;
    Settings settings{};
    size_t time{};
    thread *threadUpdate{};
    thread *threadJsonSearch{};
    mutable ofstream logFile;
    mutable mutex logMutex;

    void trustSettings() const;
    bool checkHash(bool resetHash = false) const;

    [[nodiscard]] setFileInd unionSetFiles(const setWords& request) const;
    static setWords getUniqWords(basicString& text);
    static vector<string> getAllFilesFromDir(const basic_string<char>& dir);
    void addToLog(const string& s) const;

public:

    struct myExp : public std::exception
    {
        ErrorCodes codeExp{};
    public:
        myExp() = default;
        string dir;
        explicit myExp(ErrorCodes _codeExp) : codeExp(_codeExp){};
        explicit myExp(string  _s): codeExp(ErrorCodes::WRONGDIRRECTORY), dir(std::move(_s)){};
        void show () const;
    };

    [[nodiscard]] listAnswer getResReq(basicString& request) const;
    [[maybe_unused]] [[nodiscard]] listAnswers getResReqs(const vector<string>& requests) const;
    [[nodiscard]] size_t getTimeOfUpdate() const {return time;}
    void updateDocumentBase();

    explicit SearchServer(Settings&&  _settings);
    explicit SearchServer() = default;

    ~SearchServer()
    {
        delete  threadUpdate;
        delete  threadJsonSearch;
    }
};



