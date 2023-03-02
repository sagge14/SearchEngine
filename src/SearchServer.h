//
// Created by user on 02.02.2023.
//
#pragma once
#include "ConverterJSON.h"
#include "InvertedIndex.h"

using namespace std;

typedef string basicString;
typedef set<size_t> setFileInd;
typedef list<pair<basicString, float>> listAnswer;
typedef list<pair<listAnswer, basicString>> listAnswers;

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

    enum class ErrorCodes
    {
        NAME,
        THREADCOUNT,
        WRONGDIRRECTORY,
        NOTFILESTOINDEX
    };
    struct Word {
       /** @param count количество документов в которых втречается слово @param word*
           @param word слово из поискового запроса*
           оператор "<" перегружен для сортировки по возрастанию*/

        size_t count;
        basicString word;

        bool operator ==(const Word& other) const {return (word == other.word);}
        bool operator<(const Word& r) const {return count < r.count;}

        Word(basicString _word, size_t _count) : word(std::move(_word)), count(_count){};
        ~Word() = default;
    };
    struct RelativeIndex
    {
        /** @param max максимальная абсолютная релевантность.*
            @param sum текущая абсолютная релевантность.*
            @param getRelativeIndex функция для вычисления относительной релевантности*
            Оператор "<" перегружен для сортировки по убыванию.*/

        inline static size_t max{0};
        size_t sum{0};
        size_t fileInd{0};
        string filePath;

       float getRelativeIndex() const { return (float) sum / (float) max;}

        bool operator == (const RelativeIndex& other) const {return (sum == other.sum);}
        bool operator < (const RelativeIndex& r) const {return sum > r.sum;}

        RelativeIndex(size_t fileInd, const set<basicString>& request, const InvertedIndex* index, bool exactSearch = false);
        ~RelativeIndex() = default;
    };

    atomic<bool> work{};
    hash<string> hashRequest;
    InvertedIndex* index{};
    Settings settings{};
    size_t time{};
    thread *threadUpdate{};
    thread *threadJsonSearch{};
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
    bool checkHash(bool resetHash = false) const;
    setFileInd intersectionSetFiles(const set<basicString>& request) const;
    static set<basicString> getUniqWords(basicString& text);
    static vector<basicString> getAllFilesFromDir(const basic_string<char>& dir);
    void addToLog(const string& s) const;

public:

   /** @param getAnswer функция возвращающая результат запроса @param request виде идентификатора
       файла (пусть или индекс) и соответствующей файлу относительной релевантности.*
       @param getAllAnswers функция возвращает результаты всех запросов из @param requests.*
       @param getTimeOfUpdate функция возвращает длительность последнего обновления базы индексов*
       @param updateDocumentBase функция запуска обновления базы индексов.*
       @param showSettings функция отображения информации о текущих настройках сервера.*
       @param myExp исключения выбрасываемые сервером.*/

    struct myExp : public std::exception
    {
        ErrorCodes codeExp{};
        string dir;

    public:

        void show () const;
        explicit myExp(ErrorCodes _codeExp) : codeExp(_codeExp){};
        explicit myExp(string  _s): codeExp(ErrorCodes::WRONGDIRRECTORY), dir(std::move(_s)){};

        myExp() = default;
    };

    listAnswer getAnswer(basicString& request) const;
    listAnswers getAllAnswers(vector<string> requests) const;
    size_t getTimeOfUpdate() const;
    void updateDocumentBase();
    void showSettings() const;
    explicit SearchServer(Settings&&  settings);
    explicit SearchServer() = default;

    ~SearchServer();

    /**
    Упращенная версия конструктора @param SearchServer для тестирования некоторых функций.*/

    #if TEST_MODE == true
    explicit SearchServer(const vector<basicString>& _docPaths, bool exactSearch = false)
    {
    settings.exactSearch = exactSearch;
    index = new InvertedIndex(_docPaths);
    index->updateDocumentBase();
    }
    #endif
};



