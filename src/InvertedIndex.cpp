//
// Created by user on 01.02.2023.
//
#include "InvertedIndex.h"

void InvertedIndex::updateDocumentBase(size_t threadCount)
{
    /** Очищаем старую базу индексов формируем новую.
     во время выполнения переиндексирования устанавливаем  @param work = true. */
    work = true;
    freqDictionary.clear();
    allFilesIndexing(threadCount);
    work = false;
}

void InvertedIndex::allFilesIndexing(size_t _threadCount) {
/** Файлы индексации делятся поровну @param step между потоками, если @param  _threadCount = 0,
    то количество потоков будет равно количеству ядер процессора, если заданное из настроек количество потоков
    больше количества индексируемых файлов, то количество потоков будет равно количеству индексируемых файлов. */

    size_t size = docPaths.size();
    size_t threadCount = _threadCount ? _threadCount : thread::hardware_concurrency();
    threadCount = size < threadCount ? size : threadCount;
    size_t step = size / threadCount;

    vector<thread> vThreads;

    auto smartFileInd = [this](size_t begin, size_t end){


        for(size_t  fileInd = begin; fileInd < end; fileInd++)
            fileIndexing(fileInd);
        cout << " task " << begin << "-" << end << " :" << (std::thread::id) std::this_thread::get_id() << endl;

    };
/*
    auto begin = std::chrono::high_resolution_clock::now();
    for(size_t i = 0; i <= size - step; i += step)
        vThreads.emplace_back(smartFileInd, i, i + step);

    if(size % threadCount != 0)
        vThreads.emplace_back(smartFileInd, step * threadCount, size);

    for(auto& item:vThreads)
        item.join();

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << " no pool " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
*/
    auto oneInd = [this](size_t i)
    {
        fileIndexing(i);
    };

    auto begin = std::chrono::high_resolution_clock::now();
    thread_pool pool(12);
    for(int i = 0; i < size; i++)
        pool.add_task(oneInd,i);
    pool.wait_all();
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "pool " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

   /* begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < size; i++)
        oneInd(i);

    end = std::chrono::high_resolution_clock::now();

    std::cout << "1 thread " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
*/
}

void InvertedIndex::fileIndexing(size_t fileInd)
{
    /** Если @param  TEST_MODE = true, то @param docPaths используется не как
     * база путей индексируемых файлов, а как база самих файлов (строк)
     * Из текста удаляются все символы кроме букв и цифр,
     * все буквы приводятся к нижнему регистру
     * @param freqWordFile - карта частоты слов файла */


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
    basicString text(docPaths[fileInd]);
    #endif

    map<basicString, size_t> freqWordFile;

    transform(text.begin(), text.end(), text.begin(), [](char c){
        return isalnum(c) ? tolower(c) : (' '); });

    istringstream iss(text);

    basicString word;

    while(iss)
    {
        iss >> word;
        if (iss)
        {
            auto item = freqWordFile.find(word);

            if(item != freqWordFile.end())
                item->second++;
            else
                freqWordFile[word] = 1;
        }
    }

    if(freqWordFile.empty())
        return;

    std::lock_guard<std::mutex> myLock(mapMutex);
    for(const auto& item:freqWordFile)
        freqDictionary[item.first].insert(make_pair(fileInd, freqWordFile.at(item.first)));
}

InvertedIndex::InvertedIndex(const vector<basicString>& _docPaths) :InvertedIndex() {
    /**
    Установка путей файлов подлежащих индексации*/
    docPaths = _docPaths;
}

void InvertedIndex::addToLog(const string &s) const {

    /**
    Функция для записи информации в лог-файл*/

    char dataTime[20];
    time_t now = std::time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    std::lock_guard<std::mutex> myLock(logMutex);

    logFile.open("log.ini", ios::app);
    logFile << "[" << dataTime << "] " << s << endl;
    logFile.close();

}

mapEntry InvertedIndex::getWordCount(const string &s) {
    /** Функция возвращающая @param mapEntry - обьект типа map<size_t,size_t>,
      * где first - индекс файла, а second - количество сколько раз в это файле встрчается
      * поисковое слово (ключ карты std::map @param freqDictionary) */

    auto f = freqDictionary.find(s);
    if(f != freqDictionary.end())
        return freqDictionary.at(s);
    else
        return mapEntry {};

}
