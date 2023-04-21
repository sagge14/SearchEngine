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
    typedef map<size_t, vector<unordered_map<string,mapEntry>::iterator>> wordIts;

    template <typename TP>
    std::time_t to_time_t(TP tp)
    {
        return chrono::system_clock::to_time_t(chrono::file_clock::to_sys(tp));
    }


    class MyHashFunction {

    public:
        size_t operator()(const pair<size_t, filesystem::file_time_type>& p) const
        {
            return (p.first) ^
                   (hash<time_t>()(to_time_t(p.second)));
        }
    };

    typedef unordered_set<pair<size_t, filesystem::file_time_type>, MyHashFunction> setFiles;

    class DocPaths {

    public:
        unordered_map<size_t,string> docPaths;
        setFiles docPaths2;

    public:
        string at(size_t n) const {
            return docPaths.at(n);
        }

        size_t size() const {
            return docPaths.size();
        }
        void updateSet()
        {
            docPaths2.clear();
            for(const auto& i:docPaths)
                if(filesystem::exists(i.second) && filesystem::is_regular_file(i.second))
                {
                    try {
                        docPaths2.insert(make_pair(i.first, filesystem::last_write_time(i.second)));
                    }
                    catch(...){}
                }
        }

        auto getForAdd(const vector<string>& vecDoc)
        {

           auto myCopy = [](const setFiles& first, const setFiles& second, setFiles& result)
           {
               copy_if(first.begin(),first.end(),std::inserter(result, result.end()),[&second]
                       (const auto& item){return !second.contains(item);});
           };

            setFiles newFiles,add,chg;

            for(const auto& i:vecDoc)
            {
                try{newFiles.insert(make_pair(std::hash<string>()(i), filesystem::last_write_time(i)));}
                catch(...){cout << "Ошибка тут NewFiles" << endl;}
            }

            auto t1 = chrono::high_resolution_clock::now();

            thread tAdd([&]() { myCopy(newFiles, this->docPaths2, add); });
            thread tDel([&]() { myCopy(this->docPaths2, newFiles, chg); });
            tAdd.join();
            tDel.join();

            auto t2 = chrono::high_resolution_clock::now();


            cout << std::chrono::duration_cast<chrono::microseconds>(t2-t1).count() << " sets " << endl;
            cout << "chg " << chg.size() << endl;
            cout << "add " << add.size() << endl;

           return std::move(make_tuple(add,chg));
        }

        DocPaths() = default;
    };

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

    class InvertedIndex {

        /** @param work для проверки выполнения в текущий момент времени переиндексации базы @param freqDictionary.
            @param freqDictionary база индексов.
            @param docPaths пути индексируемых файлов.
            @param mapMutex мьютекс для разделенного доступа потоков к базе индексов @param freqDictionary, во время
            выполнения индексирования базы.*
            @param logMutex мьютекс для разделения доступа между потоками к файлу @param logFile.
            @param logFile файл для хранения информации о работе сервера. */

        atomic<bool> work{};
        DocPaths docPaths;
        mutable mutex mapMutex;
        mutable mutex logMutex;
        mutable ofstream logFile;
        unordered_map<string, mapEntry> freqDictionary;
        wordIts wordIts;

        friend class search_server::SearchServer;
        friend class search_server::RelativeIndex;

        /** @param fileIndexing функция индексирования одного файла.
            @param allFilesIndexing функция индексирования всех файлов из  @param docPaths.
            @param addToLog функция добавление в @param logFile инфрормации о работе сервера. */

        void fileIndexing(size_t _fileHash);

        void allFilesIndexing(size_t threadCount = 0);

        void addToLog(const string &_s) const;

    public:

        /** @param setDocPaths для установки путей файлов подлежащих индексации.
            @param updateDocumentBase функция процесс обновления базы индексов.
            @param addToLog функция добавление в @param logFile инфрормации о работе сервера.
            @param getWordCount функция возвращающая mapEntry - обьект типа map<size_t,size_t>
            где first - это индекс файла, а second - количество сколько раз @param word
            содержится в файле с индексом first - функция используется только для тестирования */

        void setDocPaths(vector<string> _docPaths);

        void updateDocumentBase(const vector<string>& vecPaths, size_t threadCount = 0);

        mapEntry getWordCount(const string &_s);

        InvertedIndex() = default;

        explicit InvertedIndex(const vector<string> &_docPaths);

        ~InvertedIndex() = default;
    };

}

