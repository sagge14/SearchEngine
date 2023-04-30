//
// Created by user on 01.02.2023.
//
#include "ThreadPool.h"
#include "InvertedIndex.h"

void inverted_index::InvertedIndex::updateDocumentBase(const vector<string>& _vecPaths, size_t _threadCount)
{
    /** @param ind - set с идентификаторами файлов(хэши путей) подлежащих индексации
     * @param del - set с идентификаторами файлов(хэши путей) подлежащих удалению из словаря
     Во время выполнения переиндексирования устанавливаем  @param work = true */
    work = true;

    setLastWriteTimeFiles ind,del;
    std::tie(ind, del) = docPaths.getUpdate(_vecPaths);

    delFromDictionary(del);
    addToDictionary(ind, _threadCount);

    work = false;
}

void inverted_index::InvertedIndex::addToDictionary(const setLastWriteTimeFiles& _ind, size_t _threadCount) {
/**   Функция исключения файлов из базы индексов.
      Индексирование файлов осуществляется в пуле потоков, количество потоков в пуле опеределяется
      @param _threadCount. - если параметр пуст то выбирается по количеству ядер процессора,
      если параметр больше количества файлов, то приравнивается количеству файлов.
      В сете _ind содержутся индексы (хэши) путей новых, измененных и удаленных файлов, чтобы не индексировать удаленные
      файлы делается проверка *1 */

    size_t threadCount = _threadCount ? _threadCount : thread::hardware_concurrency();
    threadCount = docPaths.size() < threadCount ? docPaths.size() : threadCount;

    size_t newFileCount = _ind.size();
    #ifdef TEST_MODE
    threadCount = 1;
    #endif

    auto oneInd = [this](size_t _hashFile)
    {
            fileIndexing(_hashFile);
    };

    thread_pool pool(threadCount);

    for(auto i:_ind)
        if(filesystem::is_regular_file(docPaths.mapHashDocPaths.at(i.first))) // (*1)
            pool.add_task(oneInd,i.first);
        else
            --newFileCount;

    pool.wait_all();
    addToLog("Indexing new\t\t" + to_string(newFileCount) + "\tfiles");
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
            dictionaryIterators[_fileHash].push_back(freqDictionary.find(item.first));
        }
}

void inverted_index::InvertedIndex::addToLog(const string& _s) const {

    /**
    Функция для записи информации в лог-файл*/

    char dataTime[20];
    time_t now = time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    lock_guard<mutex> myLock(logMutex);

    logFile.open("log.log", ios::app);
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

void inverted_index::InvertedIndex::delFromDictionary(const inverted_index::setLastWriteTimeFiles& _del) {
    /** Функция исключения файлов из базы индексов */

    for(const auto& f:_del)
    {
        for(auto& i:dictionaryIterators[f.first])
        {
            i->second.erase(f.first);
            if(i->second.size() == 0)
                freqDictionary.erase(i->first);
        }
        dictionaryIterators.erase(f.first);
    }
    addToLog("Delete (change)\t" + to_string(_del.size()) + "\tfiles");
}

inverted_index::InvertedIndex &inverted_index::InvertedIndex::getInstance() {
    /** @param getInstance функция глобального обращения к единственному экземпляру класса SearchServer,
    взято у Мейерса*/
    static InvertedIndex instance;
    return instance;
}

std::string inverted_index::DocPaths::at(size_t _hashFile) const {
    /** Функция возвращающая полный путь к индексируемую файлу по его хэшу*/
    return mapHashDocPaths.at(_hashFile);
}

size_t inverted_index::DocPaths::size() const {
    /** Функция возвращающая количество индексируемых файлов*/
    return mapHashDocPaths.size();
}

std::tuple<inverted_index::setLastWriteTimeFiles, inverted_index::setLastWriteTimeFiles>
        inverted_index::DocPaths::getUpdate(const std::vector<std::string> &vecDoc) {

    /** Функция возвращающая количество индексируемых файлов*/

    setLastWriteTimeFiles newFiles,del,ind,indBuf;

    auto myCopy = [](const setLastWriteTimeFiles& first,
                     const setLastWriteTimeFiles& second, setLastWriteTimeFiles& result)
    {
        copy_if(first.begin(),first.end(),inserter(result, result.end()),[&second]
                (const auto& item){return !second.contains(item);});
    };

    mapHashDocPaths.clear();

    for(const auto& path:vecDoc)
    {
        #ifndef TEST_MODE
        size_t pathHash = std::hash<string>()(path);
        #endif
        #ifdef TEST_MODE
        static int i = 0;
        #endif
        try {
            #ifndef TEST_MODE
            newFiles.insert(make_pair(pathHash, filesystem::last_write_time(path)));
            mapHashDocPaths[pathHash] = path;
            #endif
            #ifdef TEST_MODE
            newFiles.insert(make_pair(i, filesystem::file_time_type{}));
            mapHashDocPaths[i] = path;
            i++;
            #endif
        }
        catch(...){}
    }

    thread tInd([&]() { myCopy(newFiles, lastWriteTimeFiles, indBuf); });
    thread tDel([&]() { myCopy(lastWriteTimeFiles, newFiles, del); });
    tInd.join();
    tDel.join();

    for(const auto i:del)
        if(mapHashDocPaths.contains(i.first))
            indBuf.insert(i);

    std::set_difference(indBuf.begin(),indBuf.end(),del.begin(),del.end(),
                        inserter(ind,ind.begin()));

    lastWriteTimeFiles = std::move(newFiles);

    return std::move(make_tuple(ind, del));
}
