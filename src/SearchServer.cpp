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

setFileInd SearchServer::intersectionSetFiles(const set<basicString> &request) const {

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

    if(!settings.exactSearch)
    {
        for(const auto& w: wordList)
        {
            auto sSet = getSetFromMap(w.word);
            result.insert(sSet.begin(),sSet.end());
        }

        return result;
    }

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
     * std::set  состоящий из уникальных слов запроса (разделитель пробел)
     * содержащих только буквы и цифры.*
     * Далее с помощью функции @param intersectionSetFiles находятся все файлы удовлетворяющие поисковому запросу,
     * для каждого из них расчитывается абсолютная релевантность в процессе составления списка @param Results
     * обьектов @param RelativeIndex - перед составлением списка статическая переменная @param max -
     * максимальная абсолютная релевантность - обнуляется.
     * Далее список @param Results сортируется по убыванию абсолютной релевантности и из него формируется
     * окончательный ответ @param out типа @param listAnswer  в виде списка пар идентификаторов документа (путь
     * или индекс в зависимости от настроек сервера) и относительной релевантности,
     * ограниченного максимальным размером ответа  @param maxResponse.
     * */

    if(index->work)
        std::cout << "Update base is running, pls wait!!!" << endl;
    while(index->work)
        this_thread::sleep_for(std::chrono::seconds(2));

    set<basicString> request = getUniqWords(_request);
    list<RelativeIndex> Results;
    listAnswer out;

    RelativeIndex::max = 0;

    for(const auto& fileInd: intersectionSetFiles(request))
        Results.emplace_back(fileInd, request, index,settings.exactSearch);

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

set<basicString> SearchServer::getUniqWords(basicString& text)
{
    /**
    Функция разбиения строки @param text на std::set слов.*/

    transform(text.begin(), text.end(), text.begin(), [](char c){
        return isalnum(c) ? tolower(c) : (' '); });

    istringstream iss(text);
    basicString word;
    set<basicString> out;

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
    Формируем лист ответов на все запросы, с возможностью выбора, что использовать в качестве
     идентификатора файла - индекс или текст запроса.*/

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
    /**
    Запускаем обновление базы индексов, записываем в @param time сколько времени уйдет на индексацию. */

    time = (perf_timer<chrono::seconds>::duration([this] (){
           this->index->updateDocumentBase(settings.threadCount); })
           ).count();
}

SearchServer::SearchServer(Settings&& _settings) :  time{}, index()  {
    /**
    В конструкторе сервера импортируется настройки потом они проверяются функцией @param trustSettings
     на корректность, в случае успеха запускаются 2 потока:
     1. для переодического обновления базы индексов
     2. для переодического выполнения запросов, по умолчанию из файла "Request.json"
     Информаци об работе сервера записывается в лог-файл*/

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

    /**
    Функция проверяет корректность настроек сервера:
     1. имя сервера не может быть пустым.
     2. количество потоков индексирующих базу не может быть отрицательным
     3. Файлы для индексирования должны браться либо из папки указанной в @param 'settings.dir'
     либо напрямую из файла настроек сервера (по умолчанию Settings.json)
     Если настройки не корректны выбрасывается соответствующее исключение
     */

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

    /**
    Функция сравнения хешей очередного и последнего запроса*/

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
        textRequest = basicString ((istreambuf_iterator<char>(jsonFileRequests)), (istreambuf_iterator<char>()));
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
    /**
    Функция для записи информации работ сервера в лог-файл*/

    char dataTime[20];
    time_t now = std::time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    std::lock_guard<std::mutex> myLock(logMutex);
    logFile.open("log.ini", ios::app);
    logFile << "[" << dataTime << "] " << s << endl;
    logFile.close();
}

SearchServer::~SearchServer() {

    delete  index;
    delete  threadUpdate;
    delete  threadJsonSearch;

}

void SearchServer::showSettings() const {
    settings.show();
}

size_t SearchServer::getTimeOfUpdate() const {
    return time;
}

SearchServer::RelativeIndex::RelativeIndex(size_t _fileInd, const set<basicString> &request, const InvertedIndex* index, bool exactSearch) : fileInd(_fileInd)
{
    /**
    ОБЯЗАТЕЛЬНО РАСПИСАТЬ!!!*/

    filePath = index->docPaths.at(fileInd);

    for(const auto& word:request)
    {
        if(!exactSearch && index->freqDictionary.find(word) != index->freqDictionary.end()
            && index->freqDictionary.find(word)->second.find(fileInd) != index->freqDictionary.find(word)->second.end())
         sum += index->freqDictionary.at(word).at(fileInd);
    }


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
