//
// Created by user on 01.02.2023.
//
//  std:: setlocale(LC_ALL, "Russian");
#include "InvertedIndex.h"
#include <iostream>
#include <utility>

void InvertedIndex::updateDocumentBase(size_t threadCount)
{
    work = true;
    freqDictionary.clear();
    allFilesIndexing(threadCount);
    work = false;
}

void InvertedIndex::allFilesIndexing(size_t _threadCount) {

    size_t size = docPaths.size();
    size_t threadCount = _threadCount ? _threadCount : thread::hardware_concurrency();
    threadCount = size < threadCount ? size : threadCount;
    size_t step = size / threadCount;

    vector<thread> vThreads;

    auto smartFileInd = [this](size_t begin, size_t end){
        for(size_t  fileInd = begin; fileInd < end; fileInd++)
            fileIndexing(fileInd);};

    for(size_t i = 0; i <= size - step; i += step)
        vThreads.emplace_back(smartFileInd, i, i + step);

    if(size % threadCount != 0)
        vThreads.emplace_back(smartFileInd, step * threadCount, size);

    for(auto& item:vThreads)
        item.join();

}

void InvertedIndex::fileIndexing(size_t fileInd)
{

    #if TEST_MODE == false
    ifstream file(docPaths.at(fileInd));

    if(!file.is_open())
    {
        addToLog("File " + docPaths.at(fileInd) + " - not found!");
        return;
    }

    basicString text((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
    file.close();
    #endif

    #if TEST_MODE == true
    basicString text(docPaths.at(fileInd));
    #endif

    map<basicString, size_t> freqWordDoc;

    transform(text.begin(), text.end(), text.begin(), [](char c){
        return isalnum(c) ? tolower(c) :  (' '); });

    istringstream iss(text);

    basicString word;

    while(iss)
    {
        iss >> word;
        if (iss)
        {
            auto item = freqWordDoc.find(word); //   freqWordDoc - карта частоты слов документа

            if(item != freqWordDoc.end())
                item->second++;
            else
                freqWordDoc[word] = 1;
        }
    }

    if(freqWordDoc.empty())
        return;

    std::lock_guard<std::mutex> myLock(mapMutex);
    for(const auto& item:freqWordDoc)
        freqDictionary[item.first].insert(make_pair(fileInd, freqWordDoc.at(item.first)));
}

InvertedIndex::InvertedIndex(const vector<basicString>& _docPaths) :InvertedIndex() {
    docPaths = _docPaths;
}

void InvertedIndex::addToLog(const string &s) const {

    char dataTime[20];
    time_t now = std::time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    std::lock_guard<std::mutex> myLock(logMutex);
    logFile.open("log.ini", ios::app);
    logFile << "[" << dataTime << "] " << s << endl;
    logFile.close();

}

mapEntry InvertedIndex::getWordCount(const string &s) {

    auto f = freqDictionary.find(s);
    if(f != freqDictionary.end())
        return freqDictionary.at(s);
    else
        return mapEntry {};

}
