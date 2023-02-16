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
#include <iterator>
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "Settings.h"

using namespace std;

typedef string basicString;
typedef set<basicString> setWords;
typedef set<size_t> setFileInd;
typedef list<tuple<string, float>> listAnswer;
typedef list<tuple<listAnswer, string>> listAnswers;

class SearchServer {

private:

    /** @param ErrorCodes класс перечеслений кодов ошибок сервера.*/
    enum class ErrorCodes
    {
        NAME,
        THREADCOUNT,
        WRONGDIRRECTORY,
        NOTFILESTOINDEX
    };

    /** @param Word структура, используемая для сортировки слов поиского запроса по количеству
     * документов, в которых они встречаются*/
    struct Word {
        /** @param count количество документов в которых втречается слово @param  word*/
        size_t count;
        /** @param word слово из поискового запроса*/
        basicString word;

        bool operator ==(const Word& other) const {return (word == other.word);}
        /** оператор "<" перегружен для сортировки по возрастанию*/
        bool operator<(const Word& r) const {return count < r.count;}

        Word(basicString _word, size_t _count) : word(std::move(_word)), count(_count){};
        ~Word() = default;
    };

    /** @param RelativeIndex структура для хранения информации о файле удовлетворяющем поисковому запросу*/
    struct RelativeIndex
    {
        /** @param max максимальная абсолютная релевантность.*
            @param sum текущая абсолютная релевантность.*/

        inline static size_t max{0};
        size_t sum{0};
        size_t fileInd{0};
        string filePath;

        /** @param getRelativeIndex функция для вычисления относительной релевантности*
        Оператор "<" перегружен для сортировки по убыванию.*/

        [[nodiscard]] float getRelativeIndex() const { return (float) sum / (float) max;}

        bool operator == (const RelativeIndex& other) const {return (sum == other.sum);}
        bool operator < (const RelativeIndex& r) const {return sum > r.sum;}

        RelativeIndex(size_t fileInd, const setWords& request, const InvertedIndex* index);
        ~RelativeIndex() = default;
    };

    /** @param work для проверки выполнения в текущий момент времени запроса.*
      * @param hashRequest хэш последнего выполненного запроса,
      * для проверки выполнялся ли уже данный завпрос (сравнение только с предыдущим запросом).*
      * @param time хранит длительность последнего индексирования файлов.*
      * @param threadUpdate поток запускающий переодическое обновление базы индексов.*
      * @param threadJsonSearch поток запускающий обработку запросов из файла "Request.json".*
      * @param logFile файл для хранения информации о работе сервера.*
      * @param logMutex мьютекс для разделения доступа между потоками к файлу @param logFile.*/

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
      * @param checkHash функция для сравнения хэша последнего и очередного запроса.*
      * @param intersectionSetFiles функции возвращающая результат пересечения
      * множеств (std::set) файловых индексов файлов содержащих слова из запроса.*
      * @param getUniqWords функция разбиения строки @param text на множество (std::set) слов.*
      * @param getAllFilesFromDir функция получения всех файлов в папке @param dir и во всех ее подпапках.*
      * @param addToLog функция добавление в @param logFile инфрормации о работе сервера.*/

    void trustSettings() const;
    bool checkHash(bool resetHash = false) const;
    [[nodiscard]] setFileInd intersectionSetFiles(const setWords& request) const;
    static setWords getUniqWords(basicString& text);
    static vector<basicString> getAllFilesFromDir(const basic_string<char>& dir);
    void addToLog(const string& s) const;

public:

    /** @param myExp исключения выбрасываемые сервером.*/
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

    /** @param getAnswer функция возвращающая резултат запроса @param request виде идентификатора
      * файла (пусть или индекс) и соответствующей файлу относительной релевантности.*
      * @param getAllAnswers функция возвращает результаты всех запросов из @param requests.*
      * @param getTimeOfUpdate функция возвращает длительность последнего обновления базы индексов*
      * @param updateDocumentBase функция запуска обновления базы индексов.*
      * @param showSettings функция отображения информации о текущих настройках сервера.*/

    [[nodiscard]] listAnswer getAnswer(basicString& request) const;
    [[nodiscard]] listAnswers getAllAnswers(vector<string> requests) const;
    [[nodiscard]] size_t getTimeOfUpdate() const;
    void updateDocumentBase();
    void showSettings() const;

    explicit SearchServer(Settings&&  settings);
    explicit SearchServer() = default;

    ~SearchServer();
};



