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
#include "Settings.h"
using namespace std;

typedef string basicString;
typedef map<size_t,size_t> mapEntry;

template <typename Time = chrono::seconds, typename Clock = chrono::high_resolution_clock>
struct perf_timer
{
    /** Стркуктура повзволяющая получить время выполнения любой функции.
     * Взято из книги "Решение задач на современном C++" Мариуса Бансила.
     * */
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
    map<basicString, mapEntry> freqDictionary;
    vector<basicString> docPaths;
    mutable mutex mapMutex;
    mutable mutex logMutex;
    mutable ofstream logFile;

    friend class SearchServer;

    void fileIndexing(size_t fileInd);
    void allFilesIndexing(size_t threadCount = 0);
    void addToLog(const string& s) const;

public:

    [[maybe_unused]] void setDocPaths(vector<basicString> _docPaths) {docPaths = std::move(_docPaths);};
    void updateDocumentBase(size_t threadCount = 0);
    mapEntry getWordCount(const string& s);

    InvertedIndex() = default;
    explicit InvertedIndex(const vector<basicString>& _docPaths);

    ~InvertedIndex() = default;

};


