//
// Created by user on 01.02.2023.
//
#pragma once

#include <map>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <fstream>
#include <filesystem>

namespace search_server
{
    class SearchServer;
    struct RelativeIndex;
}

namespace inverted_index {

    using namespace std;

    class hashFunction;
    typedef unordered_map<size_t, size_t> mapEntry;
    typedef unordered_map<size_t, vector<unordered_map<string,mapEntry>::iterator>> mapDictionaryIterators;
    typedef unordered_set<pair<size_t, filesystem::file_time_type>, hashFunction> setLastWriteTimeFiles;

    class hashFunction {
        /** класс хзш функции для unordered_map
        */
        time_t toTime_t(filesystem::file_time_type tp) const
        {
            return chrono::system_clock::to_time_t(chrono::file_clock::to_sys(tp));
        }
    public:
        size_t operator()(const pair<size_t, filesystem::file_time_type>& p) const
        {
            return p.first ^ hash<time_t>()(toTime_t(p.second));
        }
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

    class DocPaths {

        /** @param mapHashDocPaths карта для хранения путей индексируемых файлов по их хэшу (ключу карты)
            @param lastWriteTimeFiles сет из пар хэш пути файла, время его последнего изменения,
            для отслеживания изменений индексируемых файлов*/

   public:
       unordered_map<size_t,string> mapHashDocPaths;
       setLastWriteTimeFiles lastWriteTimeFiles;

   public:
        /** @param at функция получения пути файла по его хэшу
            @param size функция получения колечества всех файлов подлежащих индексации
            @param getUpdate функция производящая обновление списка файлов подлежащих индексируванию, отслеживания
            поввления новых файлов и изменения/удаления старых*/

       string at(size_t hashFile) const;
       size_t size() const;
       tuple<setLastWriteTimeFiles, setLastWriteTimeFiles> getUpdate(const vector<string>& vecDoc);
   };

   class InvertedIndex {

       /** @param work для проверки выполнения в текущий момент времени переиндексации базы 'freqDictionary'
           @param freqDictionary база индексов
           @param docPaths каласс для хранения, доступа и управления путями индексируемых файлов
           @param mapMutex мьютекс для разделенного доступа потоков к базе индексов 'freqDictionary', во время
           выполнения индексирования *
           @param logMutex мьютекс для разделения доступа между потоками к файлу 'logFile'
           @param logFile файл для хранения информации о работе сервера
           @param dictionaryIterators карта где ключ - индекс (хэш) файла, по которому хранится вектор итераторов
           карты 'freqDictionary' содержащих данный индекс (используется при обновлении базы индексов)
           */

        atomic<bool> work{};
        DocPaths docPaths;
        mutable mutex mapMutex;
        unordered_map<string, mapEntry> freqDictionary;
        mapDictionaryIterators dictionaryIterators;

        friend class search_server::SearchServer;
        friend class search_server::RelativeIndex;

        /** @param fileIndexing функция индексирования одного файла
            @param addToDictionary функция включения файлов в базу индексов
            @param delFromDictionary функция исключения файлов из базы индексов */

        void fileIndexing(size_t fileHash);
        void addToDictionary(const setLastWriteTimeFiles& ind, size_t threadCount = 0);
        void delFromDictionary(const setLastWriteTimeFiles& del);

        InvertedIndex(const InvertedIndex &) = delete;
        InvertedIndex(InvertedIndex &&) = delete;
        InvertedIndex& operator=(const InvertedIndex &) = delete;
        InvertedIndex& operator=(InvertedIndex &&) = delete;
        InvertedIndex() = default;

    public:

        /**
            @param updateDocumentBase функция запускающая процесс обновления базы индексов.
            @param getWordCount функция возвращающая mapEntry - обьект типа map<size_t,size_t>
            где first - это индекс файла, а second - количество сколько раз 'word'
            @param getInstance функция глобального обращения к единственному экземпляру класса SearchServer,
            взято у Мейерса*/

        void updateDocumentBase(const vector<string>& vecPaths, size_t threadCount = 0);
        mapEntry getWordCount(const string& word);
        static InvertedIndex& getInstance();
    };

}

