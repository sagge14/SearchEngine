//
// Created by user on 02.02.2023.
//
#include "SearchServer.h"

vector<string> SearchServer::getAllFilesFromDir(const string& dir) {

    /** Функция получения всех файлов из дирректории @param dir исключая имена папок */

    namespace fs = std::filesystem;
    auto recursiveGetFileNamesByExtension = [](const string &path)
    {
        vector<string> _docPaths;
        for(auto& p: fs::recursive_directory_iterator(path))
             if(p.is_regular_file()
             && p.path().extension().compare("txt") != 0
             )
            _docPaths.push_back(p.path().string());

        return _docPaths;
    };
    return recursiveGetFileNamesByExtension(dir);

}

setFileInd SearchServer::intersectionSetFiles(const setWords &request) const {

    /** В основе работы функции математическое понятие пересечения множеств (std::set)
     * В начале мы проверяем каждое слово из запроса @param request на наличие в словаре @param freqDictionary
     * и составляем лист @param wordList обьектов @param Word который хранит в себе все слова запроса и
     * соответствующее каждому слову количество файлов где оно встречается.
     *
     * Если слово из запроса @param request не найдено в @param freqDictionary - то возвращается
     * пустое множество - "return {}" и работа функции прекращается.
     *
     * Далее @param wordList сортируется по возрастанию количества документов в которых встречается поисковое слово,
     * чтобы в начале списка были самые редкие слова, встречающиеся в наименьшем количеств документов.
     *
     * Далее выполняются пересечения множеств @param setFileInd - соответствующих каждому поисковому слову,
     * получаемых с помощью лямбда-функции @param getSetFromMap из карты (std::map) @param freqDictionary  -
     * где по ключу карты (поисковое слово) содержится map<size_t,size_t> где first - это индекс файла, а second
     * количество сколько раз ключ карты (поисковое слово) @param freqDictionary содержится в файле с индексом first.
     *
     * Сначала выполняется пересечения самых маленьких множеств фаловых индексов @param setFileInd
     * соответствующих самым редким словам, т.к. для таких множеств самая большая вероятность непересекаемости,
     * что позволит быстрее выявить отсутствие файлов содержащих все слова поискового запроса.
     *
     * В цикле while выполняется пересечение всех множеств @param setFileInd соответствующих
     * словам поискового запроса начиная с самих маленьких т.к. мы сделали перед этим "wordList.sort()"
     * и возвращается результат этого пресечения @param result - множество @param setFileInd индексов файлов содержащих
     * все слова из поискового запроса @param request - оно может быть пустым если пересечений не найдено, значит
     * документов соджержащих все слова из запроса - нет!
     **/


    if(request.empty())
        return {};

    setFileInd result, first;
    list<Word> wordList;

    for(const auto& word:request)
        if(index->freqDictionary.find(word) != index->freqDictionary.end())
            wordList.emplace_back(word, index->freqDictionary.at(word).size());
        else
            return {};

    auto getSetFromMap = [this](const basicString& word) {
        setFileInd s;
        for(const auto& z:index->freqDictionary.at(word))
            s.insert(z.first);
        return s;
    };

    wordList.sort();

    result = getSetFromMap(wordList.front().word);

    while(true)
    {
        if(next(wordList.begin()) != wordList.end())
        {
            wordList.pop_front();
            first = getSetFromMap(wordList.front().word);
        }
        else
            return result;

        setFileInd intersection;

        set_intersection(result.begin(),result.end(),
                         first.begin(),first.end(),
                         std::inserter(intersection, intersection.end()));

        if(intersection.empty())
            return {};
        else
            result = intersection;
    }
}

listAnswer SearchServer::getAnswer(basicString& _request) const {

    /** Сначала с помощью атомарной булевой переменной @param work проверяем не осущетвляется ли в данный
     * момент времени обновление базы индексов (переиндексация файлов заданных в настройках сервера).
     * Если работа по переиндексации выполняется то выдаем предупреждающее собщение и раз в 2 секунды
     * проверяем закончилась ли работа по переиндексации и далее выполняем запрос.
     * Для выполнения запроса @param _request сначала получаем с помощью функции @param getUniqWords
     * std::set @param setWords состоящий из уникальных слов запроса (разделитель пробел)
     * содержащих только буквы и цифры.*
     * Далее с помощью функции @param intersectionSetFiles находятся все файлы удовлетворяющие поисковому запросу,
     * для каждого из них расчитывается абсолютная релевантность в процессе составления списка @param Results
     * обьектов @param RelativeIndex - перед составлением списка статическая переменная @param max -
     * максимальная абсолютная релевантность - обнуляется.
     * Далее список @param Results сортируется по убыванию абсолютной релевантности и из него формируется
     * окончательный ответ @param out типа @param listAnswer  в виде списка пар идентификаторов документа (путь
     * или индекс в зависимости от настроек сервера) и относительной релевантности,
     * ограниченного максимальным размером ответа  @param maxResponse.
     *
     * */
    if(index->work)
        std::cout << "Update base is running, pls wait!!!" << endl;
    while(index->work)
        this_thread::sleep_for(std::chrono::seconds(2));

    setWords request = getUniqWords(_request);
    list<RelativeIndex> Results;
    vector<RelativeIndex> Results2;
    listAnswer out;

    RelativeIndex::max = 0;

    for(const auto& fileInd: intersectionSetFiles(request))
    {
        Results.emplace_back(fileInd,request,index);
        Results2.emplace_back(fileInd,request,index);
    }


//    size_t time2 = (perf_timer<chrono::microseconds>::duration([&Results] (){
//        Results.sort(); })
//    ).count();
//
//    size_t time3 = (perf_timer<chrono::microseconds>::duration([&Results2] (){
//        sort(Results2.begin(),Results2.end(), [](auto& r, auto& r2){return r.sum < r2.sum;}); })
//    ).count();


    Results.sort();

    int i = 0;
    for(const auto& r:Results)
    {
        if(settings.dir.empty())
            out.emplace_back(to_string(r.fileInd),r.getRelativeIndex());
        else
            out.emplace_back(r.filePath,r.getRelativeIndex());

        i++;
        if(i == settings.maxResponse)
            return out;
    }
    return out;
}

setWords SearchServer::getUniqWords(basicString& text)
{

    transform(text.begin(), text.end(), text.begin(), [](char c){
        return isalnum(c) ? tolower(c) : (' '); });

    istringstream iss(text);
    basicString word;
    setWords out;

    while(iss)
    {
        iss >> word;
        if (iss)
            out.insert(word);
    }

    return out;
}

listAnswers SearchServer::getAllAnswers(vector<string> requests) const {

    /**
    Обновить или заполнить базу документов, по которой будем совершать поиск
    @param out содержимое документов
    **/
    listAnswers out;

    int i = 1;
    for(auto& request: requests)
        if(settings.requestText)
            out.push_back(make_tuple(getAnswer(request), request));
        else
        {
            string nRequest = i < 10 ? "00" + to_string(i) : i < 100 ? "0" + to_string(i) : to_string(i);
            out.push_back(make_tuple(getAnswer(request), "request" + nRequest));
            i++;
        }

    addToLog(to_string(requests.size()) + " requests processed!");

    return out;
}

void SearchServer::updateDocumentBase() {

    time = (perf_timer<chrono::seconds>::duration([this] (){
           this->index->updateDocumentBase(settings.threadCount); })
           ).count();

}

SearchServer::SearchServer(Settings&& _settings) :  time{}, index()  {

    settings = (std::move(_settings));
    trustSettings();

    addToLog("Server " + settings.name + " version " + settings.version + " is running!");

    if(settings.dir.empty())
        index = new InvertedIndex(settings.files);
    else
        index = new InvertedIndex(getAllFilesFromDir(settings.dir));

    auto periodUpdate = [this]()
    {
        while(true)
        {
            if(!work)
            {

                addToLog("Index database update started!");

                this->updateDocumentBase();
                time = getTimeOfUpdate();

                addToLog("Index database update completed! "+ to_string(index->docPaths.size()) + " files, "
                    + to_string(index->freqDictionary.size()) + " uniq words in dictionary. " + "Time of update "
                    + to_string(time) + " seconds.");

                checkHash(true);
            }
            else
            {
                this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }
            this_thread::sleep_for(std::chrono::seconds(settings.indTime));
        }

    };
    auto periodJsonSearch = [this]()
    {
        while(true)
        {
            if(!index->work && !index->freqDictionary.empty() && checkHash())
            {
                work = true;
                try
                {
                    ConverterJSON::putAnswers(this->getAllAnswers(ConverterJSON::getRequests()));
                }
                catch(...){}
                work = false;
            }
            this_thread::sleep_for(std::chrono::milliseconds (settings.searchTime));
        }
    };

    threadUpdate = new thread(periodUpdate);
    threadJsonSearch = new thread(periodJsonSearch);
    threadUpdate->detach();
    threadJsonSearch->detach();
}

void SearchServer::trustSettings() const {

    auto allFilesNotExist = [this](){
        return all_of(settings.files.begin(),settings.files.end(), [](auto path) {
                return !filesystem::exists(filesystem::path(path));});
        };

    if(settings.name.empty())
        throw(myExp(ErrorCodes::NAME));
    if(settings.threadCount < 0)
        throw(myExp(ErrorCodes::THREADCOUNT));
    if(!settings.dir.empty() && !filesystem::exists(filesystem::path(settings.dir)))
        throw(myExp(settings.dir));
    if(settings.dir.empty() && (settings.files.empty() || allFilesNotExist()))
        throw(myExp(ErrorCodes::NOTFILESTOINDEX));
}

bool SearchServer::checkHash(bool resetHash) const {

    static size_t hash{0};
    size_t newHash;

    if(resetHash)
    {
        hash = 0;
        return{};
    }

    string textRequest;
    std::ifstream jsonFileRequests("Requests.json");

    if(jsonFileRequests.is_open())
    {
            jsonFileRequests >> textRequest;
            jsonFileRequests.close();
    }
    else
        return false;

    newHash = hashRequest(textRequest);
    auto check = newHash != hash;
    hash = newHash;

    return check;
}

void SearchServer::addToLog(const string &s) const {

    char dataTime[20];
    time_t now = std::time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    std::lock_guard<std::mutex> myLock(logMutex);
    logFile.open("log.ini", ios::app);
    logFile << "[" << dataTime << "] " << s << endl;
    logFile.close();
}

SearchServer::~SearchServer() {

    delete index;
    delete  threadUpdate;
    delete  threadJsonSearch;

}

void SearchServer::showSettings() const {
    settings.show();
}

size_t SearchServer::getTimeOfUpdate() const {
    return time;
}

SearchServer::RelativeIndex::RelativeIndex(size_t _fileInd, const setWords &request, const InvertedIndex* index) : fileInd(_fileInd)
{
    filePath = index->docPaths.at(fileInd);

    for(const auto& word:request)
        sum += index->freqDictionary.at(word).at(fileInd);

    if(sum > max)
        max = sum;
}

void SearchServer::myExp::show() const {

    if(codeExp == ErrorCodes::NAME)
        cout << "Server: settings error!" << endl;
    if(codeExp == ErrorCodes::THREADCOUNT)
        cout << "Server: thread count error!" << endl;
    if(codeExp == ErrorCodes::WRONGDIRRECTORY)
        cout << "Server: directory '" << dir << "' is not exist!" << endl;
    if(codeExp == ErrorCodes::NOTFILESTOINDEX)
        cout << "Server: no files to index!" << endl;

}
