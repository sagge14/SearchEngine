//
// Created by user on 01.02.2023.
//
#pragma once
#include <list>
#include <set>
#include <map>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

using namespace std;
typedef string basicString;

template <typename Time = chrono::seconds, typename Clock = chrono::high_resolution_clock>
struct perf_timer
{
    template <typename F, typename... Args>
    static Time duration(F&& f, Args... args)
    {
        auto start = Clock::now();
        invoke(forward<F>(f), forward<Args>(args)...);
        auto end = Clock::now();
        return chrono::duration_cast<Time>(end - start);
    }
};

class InvertedIndex {

    atomic<bool> work{};
    map<basicString, map<size_t,size_t>> freqDictionary;
    vector<basicString> docPaths;
    void fileIndexing(size_t fileInd);
    void allFilesIndexing(size_t threadCount = 0);
    void addToLog(const string& s) const;
    mutable mutex mapMutex;
    mutable mutex logMutex;
    mutable ofstream logFile;

    friend class SearchServer;

public:

    void setDocPaths(vector<basicString> _docPaths) {docPaths = std::move(_docPaths);};

    [[maybe_unused]] [[nodiscard]] vector<basicString> getDocPaths() const {return docPaths;}

    void updateDocumentBase(size_t threadCount = 0);

    InvertedIndex() = default;

    [[maybe_unused]] explicit InvertedIndex(vector<basicString> _docPaths);

};


