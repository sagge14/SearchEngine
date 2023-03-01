//
// Created by user on 08.02.2023.
//

#pragma once
#include <iostream>
#include <vector>
#include <thread>

#define TEST_MODE true

struct Settings
{
    /** @param name имя сервера.*
      * @param version версия сервера,
      * @param dir для поиска повсем файлам в дирректории включая подпапки
      * если параметр пустой, то поиск осуществляется только по файлам указанным в @param files.*
      * @param exactSearch для установки серевера в режим работы "точного поиска".*
      * @param threadCount устанавливает количество потоков осуществляющих индексирования файлов,
      * если установить значение 0 - то количество потоков будет выбрано автоматически, по количеству ядер процессора.*
      * @param indTime устанавливает период переиндексации файлов в секундах.*
      * @param searchTime устанавливает период выполнения запросов из файла ("Request.json").
      * @param maxResponse устанавливает максимальное количество ответов на запрос.*
      * @param requestText для отображения в файле ответов вместо идентификаторов запрослв текста запросов */

    std::string name;
    std::string version;
    std::string dir;
    bool exactSearch;
    int threadCount{};
    size_t indTime{};
    size_t searchTime{};
    int maxResponse{};
    bool requestText{};
    std::vector<std::string> files;

    void show() const;

    Settings& operator=(Settings&& s) noexcept ;
    Settings& operator=(const Settings& s) = default;

    Settings(const Settings& s) = default;
    Settings(Settings&& s) noexcept;
    Settings();
};

