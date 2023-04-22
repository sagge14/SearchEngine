//
// Created by user on 01.02.2023.
//
#include <iostream>
#include <utility>
#include <list>
#include <execution>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <thread>
#include <atomic>
#include "InvertedIndex.h"

void inverted_index::InvertedIndex::updateDocumentBase(const vector<string>& vecPaths, size_t _threadCount)
{
    /** Очищаем старую базу индексов формируем новую.
     во время выполнения переиндексирования устанавливаем  @param work = true. */
    work = true;

    setLastWriteTimeFiles ind,del;

    std::tie(ind, del) = docPaths.getForAdd(vecPaths);

    auto t1 = chrono::high_resolution_clock::now();

    for(const auto& f:del)
    {
        for(auto& i:wordIts[f.first])
        {
            i->second.erase(f.first);
            if(i->second.size() == 0)
                freqDictionary.erase(i->first);
        }
        wordIts.erase(f.first);
    }

    auto t2 = chrono::high_resolution_clock::now();

    cout << std::chrono::duration_cast<chrono::milliseconds>(t2-t1).count() << " dicto " << endl;

    for(const auto i:del)
        if(docPaths.docPaths.contains(i.first))
            ind.insert(i);


    allFilesIndexing(ind,_threadCount);

    cout << "dic size " << freqDictionary.size() << endl;

     work = false;
}

void inverted_index::InvertedIndex::allFilesIndexing(const setLastWriteTimeFiles& _ind, size_t _threadCount) {
/** Индексирование файлов осуществляется в пуле потоков, количество потоков в пуле опеределяется
 * @param _threadCount. - если параметр пуст то выбирается по количеству ядер процессора,
 * если параметр больше количества файлов, то приравнивается количеству файлов. */

    size_t threadCount = _threadCount ? _threadCount : thread::hardware_concurrency();
    threadCount = docPaths.size() < threadCount ? docPaths.size() : threadCount;

    auto oneInd = [this](size_t i)
    {
        fileIndexing(i);
    };

    thread_pool pool(threadCount);

    for(auto i:_ind)
        pool.add_task(oneInd,i.first);

    pool.wait_all();
}

void inverted_index::InvertedIndex::fileIndexing(size_t _fileHash)
{
    /** Если @param  TEST_MODE = true, то @param docPaths используется не как
     * база путей индексируемых файлов, а как база самих файлов (строк)
     * Из текста удаляются все символы кроме букв и цифр,
     * все буквы приводятся к нижнему регистру
     * @param freqWordFile - карта частоты слов файла */

    #ifndef TEST_MODE
    ifstream file(docPaths.at(_fileHash));

    if(!file.is_open())
    {
        addToLog("File " + docPaths.at(_fileHash) + " - not found!");
        return;
    }

    string text((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

    file.close();
    #endif

    #ifdef TEST_MODE
    string text(docPaths.at(_fileHash));
    #endif

    map<string, size_t> freqWordFile;

    transform(text.begin(), text.end(), text.begin(), [](char c){
        return isalnum(c) ? tolower(c) : (' '); });

    istringstream iss(text);
    string word;

    while(iss)
    {
        iss >> word;
        if (iss)
        {
            if(auto item = freqWordFile.find(word); item != freqWordFile.end())
                item->second++;
            else
                freqWordFile[word] = 1;
        }
    }

    if(freqWordFile.empty())
        return;

    lock_guard<mutex> myLock(mapMutex);
    for(const auto& item:freqWordFile)
        {
            freqDictionary[item.first].insert(make_pair(_fileHash, freqWordFile.at(item.first)));
            wordIts[_fileHash].push_back(freqDictionary.find(item.first));
        }
}

void inverted_index::InvertedIndex::addToLog(const string& _s) const {

    /**
    Функция для записи информации в лог-файл*/

    char dataTime[20];
    time_t now = time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    lock_guard<mutex> myLock(logMutex);

    logFile.open("log.ini", ios::app);
    logFile << "[" << dataTime << "] " << _s << endl;
    logFile.close();

}

inverted_index::mapEntry inverted_index::InvertedIndex::getWordCount(const string& _s) {
    /** Функция возвращающая @param mapEntry - обьект типа map<size_t,size_t>,
      * где first - индекс файла, а second - количество сколько раз в это файле встрчается
      * поисковое слово (ключ карты map @param freqDictionary) */

    if(auto f = freqDictionary.find(_s); f != freqDictionary.end())
        return freqDictionary.at(_s);
    else
        return mapEntry {};

}
