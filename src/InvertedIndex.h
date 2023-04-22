//
// Created by user on 01.02.2023.
//
#pragma once

#include "TreadPool.h"
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace search_server
{
    class SearchServer;
    struct RelativeIndex;
}

namespace inverted_index {

    using namespace std;

    typedef unordered_map<size_t, size_t> mapEntry;
    typedef map<size_t, vector<unordered_map<string,mapEntry>::iterator>> mapDictionaryIterators;




    class hashFunction {
        std::time_t toTime_t(filesystem::file_time_type tp) const
        {
            return chrono::system_clock::to_time_t(chrono::file_clock::to_sys(tp));
        }
    public:
        size_t operator()(const pair<size_t, filesystem::file_time_type>& p) const
        {
            return (p.first) ^
                   (hash<time_t>()(toTime_t(p.second)));
        }
    };

    typedef unordered_set<pair<size_t, filesystem::file_time_type>, hashFunction> setLastWriteTimeFiles;

    template<typename Time = chrono::seconds, typename Clock = chrono::high_resolution_clock>
    struct perf_timer {
        /** Стркуктура повзволяющая получить время выполнения любой функции.
         * Взято из книги "Решение задач на современном C++" Мариуса Бансила.
         * */
        template<typename F, typename... Args>
        static Time duration(F &&f, Args... args) {
            auto start = Clock::now();
            invoke(forward<F>(f), forward<Args>(args)...);
            auto end = Clock::now();
            return chrono::duration_cast<Time>(end - start);
        }
    };

    class DocPaths {

    public:
        unordered_map<size_t,string> docPaths;
        setLastWriteTimeFiles docPaths2;

    public:
        string at(size_t n) const {
            return docPaths.at(n);
        }

        size_t size() const {
            return docPaths.size();
        }

        auto getForAdd(const vector<string>& vecDoc)
        {
           setLastWriteTimeFiles newFiles,del,ind;

           auto myCopy = [](const setLastWriteTimeFiles& first,
                            const setLastWriteTimeFiles& second, setLastWriteTimeFiles& result)
           {
               copy_if(first.begin(),first.end(),std::inserter(result, result.end()),[&second]
                       (const auto& item){return !second.contains(item);});
           };

            for(const auto& path:vecDoc)
            {
                size_t pathHash = std::hash<string>()(path);
                try {
                    newFiles.insert(make_pair(pathHash, filesystem::last_write_time(path)));
                    docPaths[pathHash] = path;
                }
                catch(...){}
            }

            auto t1 = chrono::high_resolution_clock::now();

            thread tInd([&]() { myCopy(newFiles, docPaths2, ind); });
            thread tDel([&]() { myCopy(docPaths2, newFiles, del); });
            tInd.join();
            tDel.join();

            auto t2 = chrono::high_resolution_clock::now();

            cout << std::chrono::duration_cast<chrono::microseconds>(t2-t1).count() << " sets " << endl;
            cout << "ind " << ind.size() << endl;
            cout << "del " << del.size() << endl;

            docPaths2 = std::move(newFiles);

           return std::move(make_tuple(ind, del));
        }

        DocPaths() = default;
    };

    class InvertedIndex {

        /** @param work для проверки выполнения в текущий момент времени переиндексации базы 'freqDictionary'.
            @param freqDictionary база индексов.
            @param docPaths пути индексируемых файлов.
            @param mapMutex мьютекс для разделенного доступа потоков к базе индексов 'freqDictionary', во время
            выполнения индексирования базы.*
            @param logMutex мьютекс для разделения доступа между потоками к файлу 'logFile'.
            @param logFile файл для хранения информации о работе сервера. */

        atomic<bool> work{};
        DocPaths docPaths;
        mutable mutex mapMutex;
        mutable mutex logMutex;
        mutable ofstream logFile;
        unordered_map<string, mapEntry> freqDictionary;
        mapDictionaryIterators wordIts;

        friend class search_server::SearchServer;
        friend class search_server::RelativeIndex;

        /** @param fileIndexing функция индексирования одного файла.
            @param allFilesIndexing функция индексирования всех файлов из docPaths.
            @param addToLog функция добавление в 'logFile' инфрормации о работе сервера. */

        void fileIndexing(size_t _fileHash);

        void allFilesIndexing(const setLastWriteTimeFiles& ind, size_t threadCount = 0);

        void addToLog(const string &_s) const;

    public:

        /** @param setDocPaths для установки путей файлов подлежащих индексации.
            @param updateDocumentBase функция процесс обновления базы индексов.
            @param addToLog функция добавление в 'logFile' инфрормации о работе сервера.
            @param getWordCount функция возвращающая mapEntry - обьект типа map<size_t,size_t>
            где first - это индекс файла, а second - количество сколько раз 'word'
            содержится в файле с индексом first - функция используется только для тестирования */


        void updateDocumentBase(const vector<string>& vecPaths, size_t threadCount = 0);

        mapEntry getWordCount(const string& word);

        InvertedIndex() = default;

        ~InvertedIndex() = default;
    };

}

