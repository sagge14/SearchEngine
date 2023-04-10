//
// Created by user on 01.02.2023.
//
#pragma once
#include <iostream>
#include <utility>
#include <list>
#include <set>
#include <map>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include "TreadPool.h"
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
   /** @param work для проверки выполнения в текущий момент времени переиндексации базы @param freqDictionary.
       @param freqDictionary база индексов.
       @param docPaths пути индексируемых файлов.
       @param mapMutex мьютекс для разделенного доступа потоков к базе индексов @param freqDictionary, во время
       выполнения индексирования базы.*
       @param logMutex мьютекс для разделения доступа между потоками к файлу @param logFile.
       @param logFile файл для хранения информации о работе сервера. */

    atomic<bool> work{};
    map<basicString, mapEntry> freqDictionary;
    vector<basicString> docPaths;
    mutable mutex mapMutex;
    mutable mutex logMutex;
    mutable ofstream logFile;

    friend class SearchServer;

   /** @param fileIndexing функция индексирования одного файла.
       @param allFilesIndexing функция индексирования всех файлов из  @param docPaths.
       @param addToLog функция добавление в @param logFile инфрормации о работе сервера. */

    void fileIndexing(size_t fileInd);
    void allFilesIndexing(size_t threadCount = 0);
    void addToLog(const string& s) const;

public:

    /** @param setDocPaths для установки путей файлов подлежащих индексации.
        @param updateDocumentBase функция процесс обновления базы индексов.
        @param addToLog функция добавление в @param logFile инфрормации о работе сервера.
        @param getWordCount функция возвращающая mapEntry - обьект типа map<size_t,size_t>
        где first - это индекс файла, а second - количество сколько раз @param word
        содержится в файле с индексом first - функция используется только для тестирования */

    void setDocPaths(vector<basicString> _docPaths) {docPaths = std::move(_docPaths);};
    void updateDocumentBase(size_t threadCount = 0);
    mapEntry getWordCount(const string& word);

    InvertedIndex() = default;
    explicit InvertedIndex(const vector<basicString>& _docPaths);

    ~InvertedIndex() = default;
};


