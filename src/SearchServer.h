//
// Created by user on 02.02.2023.
//
#pragma once
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include <set>
#ifndef TEST_MODE
#include "AsioServer.h"
#endif

namespace search_server {

    using namespace std;
    typedef set <size_t> setFileInd;
    typedef list <pair<string, float>> listAnswer;
    typedef list <pair<listAnswer, string>> listAnswers;

    enum class ErrorCodes {
        NAME,
        THREADCOUNT,
        WRONGDIRRECTORY,
        NOTFILESTOINDEX,
        ASIOPORT
    };
    struct Word {
        /** @param count количество документов в которых втречается слово @param word*
            @param word слово из поискового запроса*
            оператор "<" перегружен для сортировки по возрастанию*/

        size_t count;
        string word;

        bool operator==(const Word &other) const { return (word == other.word); }
        bool operator<(const Word &r) const { return count < r.count; }

        Word(string _word, size_t _count) : word(std::move(_word)), count(_count) {};
    };
    struct RelativeIndex {
        /** @param max максимальная абсолютная релевантность.*
            @param sum текущая абсолютная релевантность.*
            @param getRelativeIndex функция для вычисления относительной релевантности*
            Оператор "<" перегружен для сортировки по убыванию.*/

        inline static size_t max{0};
        size_t sum{0};
        size_t fileInd{0};

        float getRelativeIndex() const { return (float) sum / (float) max; }
        bool operator==(const RelativeIndex &other) const { return (sum == other.sum);}
        bool operator<(const RelativeIndex &r) const { return sum > r.sum;}

        RelativeIndex(size_t fileInd, const set <string> &_request, bool _exactSearch = false);
    };
    class Settings
    {
        /** @param name имя сервера.
          * @param version версия сервера,
          * @param dir для поиска по всем файлам в дирректории включая подпапки
          * если параметр пустой, то поиск осуществляется только по файлам указанным в @param files.*
          * @param exactSearch для установки серевера в режим работы "точного поиска".*
          * @param threadCount устанавливает количество потоков осуществляющих индексирование файлов,
          * если установить значение 0 - то количество потоков будет выбрано автоматически, по количеству ядер процессора.*
          * @param indTime устанавливает период переиндексации файлов в секундах.*
          * @param maxResponse устанавливает максимальное количество ответов на запрос.*
          * @param requestText для отображения в файле ответов вместо идентификаторов запросов текста запросов */

        Settings(const Settings &) = delete;
        Settings(Settings &&) = delete;
        Settings& operator=(const Settings &) = delete;
        Settings& operator=(Settings &&) = delete;
        Settings();

    public:

        std::string name;
        std::string version;
        std::string dir;
        bool exactSearch;
        int threadCount{};
        size_t indTime{};
        int maxResponse{};
        bool requestText{};
        int asioPort = 15001;
        std::vector<std::string> files;

        static Settings& getInstance();

        void show() const;
    };
    class SearchServer {

        /** @param work для проверки выполнения в текущий момент времени запроса.*
            @param hashRequest хэш последнего выполненного запроса,
            для проверки выполнялся ли уже данный завпрос (сравнение только с предыдущим запросом).*
            @param time хранит длительность последнего индексирования файлов.*
            @param threadUpdate поток запускающий переодическое обновление базы индексов.*
            @param threadJsonSearch поток запускающий обработку запросов из файла "Request.json".*
            @param logFile файл для хранения информации о работе сервера.*
            @param logMutex мьютекс для разделения доступа между потоками к файлу @param logFile.*
            @param ErrorCodes класс перечеслений кодов ошибок сервера.*
            @param Word структура, используемая для сортировки слов поиского запроса по количеству*
            документов, в которых они встречаются*
            @param RelativeIndex структура для хранения информации о файле удовлетворяющем поисковому запросу*/

        #ifndef TEST_MODE
        boost::asio::io_context io_context;
        std::unique_ptr<asio_server::AsioServer> asioServer;
        std::unique_ptr<thread> threadUpdate;
        std::unique_ptr<thread> threadAsio;
        friend class asio_server::session;
        #endif

        size_t time{};
        mutable atomic<bool> work{};
        mutable mutex updateM;
        mutable mutex searchM;
        mutable ofstream logFile;
        mutable mutex logMutex;

        /** @param trustSettings функция проверки корректности настроек сервера.*
            @param checkHash функция для сравнения хэша последнего и очередного запроса.*
            @param intersectionSetFiles функции возвращающая результат пересечения
            множеств (std::set) файловых индексов файлов содержащих слова из запроса.*
            @param getUniqWords функция разбиения строки @param text на множество (std::set) слов.*
            @param getAllFilesFromDir функция получения всех файлов в папке @param dir и во всех ее подпапках.*
            @param addToLog функция добавление в @param logFile инфрормации о работе сервера.*/

        void trustSettings() const;
        setFileInd intersectionSetFiles(const std::set<string> &request) const;
        static std::set<string> getUniqWords(string &text);
        static vector<string> getAllFilesFromDir(const basic_string<char> &dir);
        void addToLog(const string &s) const;

        SearchServer(const SearchServer &) = delete;
        SearchServer(SearchServer &&) = delete;
        SearchServer& operator=(const SearchServer &) = delete;
        SearchServer& operator=(SearchServer &&) = delete;

        #ifndef TEST_MODE
        explicit SearchServer();
        ~SearchServer();
        #endif

    public:

        /** @param getAnswer функция возвращающая результат запроса @param request виде идентификатора
            файла (пусть или индекс) и соответствующей файлу относительной релевантности.*
            @param getAllAnswers функция возвращает результаты всех запросов из @param requests.*
            @param getTimeOfUpdate функция возвращает длительность последнего обновления базы индексов*
            @param updateDocumentBase функция запуска обновления базы индексов.*
            @param showSettings функция отображения информации о текущих настройках сервера.*
            @param myExp исключения выбрасываемые сервером.*/

        struct myExp : public std::exception {
            ErrorCodes codeExp{};
            string dir;

        public:

            void show() const;
            explicit myExp(ErrorCodes _codeExp) : codeExp(_codeExp) {};
            explicit myExp(string _s) : codeExp(ErrorCodes::WRONGDIRRECTORY), dir(std::move(_s)) {};
            myExp() = default;
        };

        listAnswer getAnswer(string &request) const;
        listAnswers getAllAnswers(vector<string> requests) const;
        size_t getTimeOfUpdate() const;
        void updateDocumentBase();
        void showSettings() const;

        static SearchServer& getInstance();

        #ifdef TEST_MODE
        explicit SearchServer(const vector<string>& _docPaths, bool exactSearch = false)
        {
            /**
            Упрощенная версия конструктора @param SearchServer для тестирования.*/

            Settings::getInstance().exactSearch = exactSearch;
            inverted_index::InvertedIndex::getInstance().updateDocumentBase(_docPaths);
        }
        #endif
    };
}


