//
// Created by user on 02.02.2023.
//
#include "SearchServer.h"


std::vector<std::string> search_server::SearchServer::getAllFilesFromDir(const string& dir) {
    /**
    Функция получения всех файлов из дирректории @param dir и ее подпапках, исключая имена папок */

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

search_server::setFileInd search_server::SearchServer::intersectionSetFiles(const set<string> &request) const {
    /**
     * Если сервер работает в режиме "точного поиска" @param 'settings.exactSearch' ==  true:
     *  В основе работы функции математическое понятие пересечения множеств (std::set)
     *  В начале мы проверяем каждое слово из запроса @param request на наличие в словаре @param freqDictionary
     *  и составляем лист @param wordList обьектов @param Word который хранит в себе все слова запроса и
     *  соответствующее каждому слову количество файлов где оно встречается.
     *
     *  Если слово из запроса @param request не найдено в @param freqDictionary - то возвращается
     *  пустое множество - "return {}" и работа функции прекращается.
     *
     *  Далее @param wordList сортируется по возрастанию количества документов в которых встречается поисковое слово,
     *  чтобы в начале списка были самые редкие слова, встречающиеся в наименьшем количеств документов.
     *
     *  Далее выполняются пересечения множеств @param setFileInd - соответствующих каждому поисковому слову,
     *  получаемых с помощью лямбда-функции @param getSetFromMap из карты (std::map) @param freqDictionary  -
     *  где по КЛЮЧУ КАРТЫ (поисковое слово) содержится map<size_t,size_t> где first - это индекс файла, а second
     *  количество сколько раз КЛЮЧ КАРТЫ @param freqDictionary содержится в файле с индексом first.
     *
     *  Сначала выполняется пересечения самых маленьких множеств фаловых индексов @param setFileInd
     *  соответствующих самым редким словам, т.к. для таких множеств самая большая вероятность непересекаемости,
     *  что позволит быстрее выявить отсутствие файлов содержащих все слова поискового запроса.
     *
     *  В цикле while выполняется пересечение всех множеств @param setFileInd соответствующих
     *  словам поискового запроса начиная с самих маленьких т.к. мы сделали перед этим "wordList.sort()"
     *  и возвращается результат этого пресечения @param result - множество @param setFileInd индексов файлов содержащих
     *  все слова из поискового запроса @param request - оно может быть пустым если пересечений не найдено, значит
     *  документов содержащих все слова из запроса - нет!
     *
     * Если сервер работает в режиме обычного поиска @param 'settings.exactSearch' ==  false
     * (необязательно все слова запроса должны содержаться в файле-результате):
     *  Просто находим все документы содержащие хотябы одно слово из запроса, обьединяем их в множество
     *  @param setFileInd @param result возвращаем результат :)
     **/
    using namespace inverted_index;
    if(request.empty())
        return {};

    setFileInd result, first;
    list<Word> wordList;

    auto getSetFromMap = [](const std::string& word) {
        setFileInd s;
        for(const auto& z:InvertedIndex::getInstance().freqDictionary.at(word))
            s.insert(z.first);
        return s;
    };

    for(const auto& word:request)
        if(InvertedIndex::getInstance().freqDictionary.find(word) != InvertedIndex::getInstance().freqDictionary.end())
            wordList.emplace_back(word, InvertedIndex::getInstance().freqDictionary.at(word).size());
        else if (!Settings::getInstance().exactSearch)
            continue;
        else
            return {};

    if(!Settings::getInstance().exactSearch)
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

listAnswer search_server::SearchServer::getAnswer(string& _request) const {
    /** Сначала с помощью атомарной булевой переменной @param work проверяем не осущетвляется ли в данный
     * момент времени обновление базы индексов (переиндексация файлов, заданных в настройках сервера).
     * Если работа по переиндексации выполняется то выдаем предупреждающее собщение и раз в 2 секунды
     * проверяем закончилась ли работа по переиндексации и далее выполняем запрос.
     * Для выполнения запроса @param _request сначала получаем с помощью функции @param getUniqWords
     * std::set  состоящий из уникальных слов запроса (разделитель пробел),
     * содержащих только буквы и цифры.*
     * Далее с помощью функции @param intersectionSetFiles находятся все файлы удовлетворяющие поисковому запросу,
     * для каждого из них расчитывается абсолютная релевантность в процессе составления списка @param Results
     * обьектов @param RelativeIndex - перед составлением списка статическая переменная @param max -
     * максимальная абсолютная релевантность - обнуляется.
     * Далее список @param Results сортируется по убыванию абсолютной релевантности и из него формируется
     * окончательный ответ @param out типа @param listAnswer  в виде списка пар идентификаторов документа (путь
     * или индекс в зависимости от настроек сервера) и относительной релевантности.
     * Размером списока ответов ограничен @param maxResponse.
     * */
    using namespace inverted_index;
    work = true;

    if(InvertedIndex::getInstance().work)
        std::cout << "Update base is running, pls wait!!!" << endl;
    updateM.lock();

    set<std::string> request = getUniqWords(_request);

    list<RelativeIndex> Results;
    listAnswer out;

    RelativeIndex::max = 0;

    for(const auto& fileInd: intersectionSetFiles(request))
        Results.emplace_back(fileInd, request, Settings::getInstance().exactSearch);

    Results.sort();

    int i = 0;
    for(const auto& r:Results)
    {
        if(Settings::getInstance().dir.empty())
            out.emplace_back(to_string(r.fileInd),r.getRelativeIndex());
        else
            out.emplace_back(InvertedIndex::getInstance().docPaths.at(r.fileInd),r.getRelativeIndex());

        i++;
        if(i == Settings::getInstance().maxResponse)
        {
            if(!updateM.try_lock())
                updateM.unlock();

            work = false;
            return out;
        }
    }

    updateM.unlock();

    work = false;
    return out;
}

std::set<std::string> search_server::SearchServer::getUniqWords(string& text) {
    /**
    Функция разбиения строки @param text на std::set слов.*/

    transform(text.begin(), text.end(), text.begin(), [](char c){
        return isalnum(c) ? tolower(c) : (' '); });

    istringstream iss(text);
    std::string word;
    set<std::string> out;

    while(iss)
    {
        iss >> word;
        if (iss)
            out.insert(word);
    }

    return out;
}

listAnswers search_server::SearchServer::getAllAnswers(vector<string> requests) const {
    /**
    Формируем лист ответов на все запросы, с возможностью выбора, что использовать в качестве
     идентификатора файла - индекс или текст запроса.*/

    listAnswers out;
    int i = 1;

    for(auto& request: requests)
        if(Settings::getInstance().requestText)
            out.push_back(make_pair(getAnswer(request), request));
        else
        {
            string nRequest = i < 10 ? "00" + to_string(i) : i < 100 ? "0" + to_string(i) : to_string(i);
            out.push_back(make_pair(getAnswer(request), "request" + nRequest));
            i++;
        }

    return out;
}

void search_server::SearchServer::updateDocumentBase() {
    /**
    Запускаем обновление базы индексов, записываем в @param time сколько времени уйдет на индексацию. */

    using namespace inverted_index;
    time = inverted_index::perf_timer<chrono::milliseconds>::duration([this]() {
        InvertedIndex::getInstance().updateDocumentBase(getAllFilesFromDir(Settings::getInstance().dir),
                                                        Settings::getInstance().threadCount);}).count();

}
#ifndef TEST_MODE
search_server::SearchServer::SearchServer() :  time{} {
    /**
     В конструкторе сервера импортируется настройки, потом они проверяются функцией @param trustSettings
     на корректность, в случае успеха запускаются 2 потока:
        1. для переодического обновления базы индексов
        2. для переодического выполнения запросов, по умолчанию из файла "Request.json"

     Потоки проверяют работу друг друга через сответствующие атомарные булевые переменные @param work:
     их работы никогда не пресекается - запрос из файла не обрабатывается пока не закончится переиндексирование базы.
     Перендексирование базы не запускается пока сервер обрабатывает запрос.
     После окончания переиндексирования базы производится сброс хеша последнего запроса функцией
     @param checkHash(true), true - как раз передается чтобы сбросить хеш. После этого последний запрос будет обработан
     заново уже на основе новой базы индексов.

     Информаци об работе сервера записывается в лог-файл.
     */

    auto periodicUpdate = [this]()
    {
        using namespace inverted_index;
        while(true)
        {
            if(work)
                std::cout << "Doing request, update later!!!" << endl;

            updateM.lock();

            this->updateDocumentBase();
            time = getTimeOfUpdate();

            addToLog("Index database update completed! "+ to_string(InvertedIndex::getInstance().docPaths.size()) + " files, "
                     + to_string(InvertedIndex::getInstance().freqDictionary.size()) + " uniq words in dictionary. " + "Time of update "
                     + to_string(time) + " milliseconds.");

            updateM.unlock();

            this_thread::sleep_for(std::chrono::seconds(Settings::getInstance().indTime));
        }
    };

    ConverterJSON::getSettings();
    trustSettings();

    addToLog("Server " + Settings::getInstance().name + " version " + Settings::getInstance().version + " is running!");

    threadUpdate = std::make_unique<thread>(periodicUpdate);
    asioServer = std::make_unique<asio_server::AsioServer>(io_context, Settings::getInstance().asioPort);
    threadAsio = std::make_unique<thread>([this](){io_context.run();});

   // threadUpdate = new thread(periodicUpdate);
 //   asioServer = new asio_server::AsioServer(io_context, Settings::getInstance().asioPort);
  //  threadAsio =  new thread([this](){io_context.run();});

    threadUpdate->detach();
    threadAsio->detach();
}
#endif
void search_server::SearchServer::trustSettings() const {
    /**
    Функция проверяет корректность настроек сервера:
     1. Имя сервера не может быть пустым.
     2. Количество потоков индексирующих базу не может быть отрицательным.
     3. Файлы для индексирования должны браться либо из папки указанной в @param 'settings.dir'
        либо напрямую из файла настроек сервера (по умолчанию Settings.json).
    Если настройки не корректны выбрасывается соответствующее исключение. */

    auto allFilesNotExist = [](){
        return all_of(Settings::getInstance().files.begin(),Settings::getInstance().files.end(), [](auto path) {
                return !filesystem::exists(filesystem::path(path));});
        };

    if(Settings::getInstance().name.empty())
        throw(myExp(ErrorCodes::NAME));
    if(Settings::getInstance().threadCount < 0)
        throw(myExp(ErrorCodes::THREADCOUNT));
    if(Settings::getInstance().asioPort > 65535 || Settings::getInstance().asioPort < 0)
        throw(myExp(ErrorCodes::ASIOPORT));
    if(!Settings::getInstance().dir.empty() && !filesystem::exists(filesystem::path(Settings::getInstance().dir)))
        throw(myExp(Settings::getInstance().dir));
    if(Settings::getInstance().dir.empty() && (Settings::getInstance().files.empty() || allFilesNotExist()))
        throw(myExp(ErrorCodes::NOTFILESTOINDEX));
}

void search_server::SearchServer::addToLog(const string &s) const {
    /**
    Функция для записи информации работе сервера в лог-файл*/

    char dataTime[20];
    time_t now = std::time(nullptr);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    std::lock_guard<std::mutex> myLock(logMutex);
    logFile.open("log.log", ios::app);
    logFile << "[" << dataTime << "] " << s + ";" << endl;
    logFile.close();
}

void search_server::SearchServer::showSettings() const {
    /**
    Для отображения текущих настроек сервера*/
    Settings::getInstance().show();
}

size_t search_server::SearchServer::getTimeOfUpdate() const {
    /**
    Для получения длительности последнего обновления базы индексов*/
    return time;
}
#ifndef TEST_MODE
search_server::SearchServer& search_server::SearchServer::getInstance() {
    static SearchServer instance;
    return instance;
}
search_server::SearchServer::~SearchServer() {

        io_context.stop();
}
#endif
search_server::RelativeIndex::RelativeIndex(size_t _fileInd, const set<string>& _request, bool _exactSearch)
{
    /**
    Т.к. сервер может работать в двух режимах: точного поиска и обычного, для которого не обязательно все слова из запроса
     должны одержаться в файле-результате, то во втором случае перед вычисление количества сколько раз слово
     встречается в файле нужно проверить есть ли вообще это слово в словаре @param freqDictionary и есть ли
     это слово в самом документе с индексом @param fileInd - эту проверку осуществляет функция @param checkWordAndFileInd.
     @param sum статическое поле класса, хранит максимальную абсолютную релевантность файла-результата, используется для
     вычисления относительной релевантности.
     */

    using namespace inverted_index;
    fileInd = _fileInd;

    auto checkWordAndFileInd =[_fileInd] (const auto& word) {
        return (InvertedIndex::getInstance().freqDictionary.find(word) != InvertedIndex::getInstance().freqDictionary.end() &&
                InvertedIndex::getInstance().freqDictionary.find(word)->second.find(_fileInd) != InvertedIndex::getInstance().freqDictionary.find(word)->second.end());
    };

    for(const auto& word:_request)
    {
        if(_exactSearch || checkWordAndFileInd(word))
         sum += InvertedIndex::getInstance().freqDictionary.at(word).at(_fileInd);
    }

    if(sum > max)
        max = sum;
}

void search_server::SearchServer::myExp::show() const {
    /**
    Сообщения о возможных ошибках в настройках сервера*/
    if(codeExp == ErrorCodes::NAME)
        cout << "Server: settings error!" << endl;
    if(codeExp == ErrorCodes::THREADCOUNT)
        cout << "Server: thread count error!" << endl;
    if(codeExp == ErrorCodes::ASIOPORT)
        cout << "Server: asio port error! Port must be > 0 and < 65 536" << endl;
    if(codeExp == ErrorCodes::WRONGDIRRECTORY)
        cout << "Server: directory '" << dir << "' is not exist!" << endl;
    if(codeExp == ErrorCodes::NOTFILESTOINDEX)
        cout << "Server: no files to index!" << endl;
}

void search_server::Settings::show() const
{
    std::cout << "--- Server information ---" << std::endl;
    std::cout << std::endl << "Name:\t\t\t\t" << name << std::endl;
    std::cout << "Version:\t\t\t" << version << std::endl;
    std::cout << "Asio port:\t\t\t" << asioPort << std::endl;
    std::cout << "Number of maximum responses:\t" << maxResponse << std::endl;
    if(!dir.empty())
        std::cout << "The directory for indexing:\t" << dir << std::endl;
    std::cout << "Thread count:\t\t\t";
    if(threadCount)
        std::cout << threadCount << std::endl;
    else
        std::cout << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Index database update period:\t" << indTime << " seconds" << std::endl;
    std::cout << "Show request as text:\t\t" << std::boolalpha << requestText << std::endl;
    std::cout << "Use exact search:\t\t" << std::boolalpha << exactSearch << std::endl << std::endl;
}

search_server::Settings::Settings() {

    name = "TestServer";
    version = "1.1";
    dir = "";
    threadCount = 1;
    maxResponse = 5;
    exactSearch = false;
}

search_server::Settings& search_server::Settings::getInstance() {

    static Settings instance;
    return instance;
}
