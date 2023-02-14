//
// Created by user on 02.02.2023.
//
#include "SearchServer.h"
#include <cassert>

vector<string> SearchServer::getAllFilesFromDir(const string& dir) {

    namespace fs = std::filesystem;
    auto recursiveGetFileNamesByExtension = [](const string &path)
    {
        vector<string> _docPaths;
        for(auto& p: fs::recursive_directory_iterator(path))
            // if(p.is_regular_file() && p.path().extension().compare(extension) == 0)
            _docPaths.push_back(p.path().string());

        return _docPaths;
    };
    return recursiveGetFileNamesByExtension(dir);

}

setFileInd SearchServer::unionSetFiles(const set<basicString> &request) const {

   // std::lock_guard<std::mutex> myLock(Mutex);

    if(request.empty())
        return {};

    setFileInd result, first;
    list<Word> wordList;


    for(const auto& word:request)
        if(index.freqDictionary.find(word) != index.freqDictionary.end())
            wordList.emplace_back(word, index.freqDictionary.at(word).size());
        else
            return {};

    auto getSetFromMap = [this](const basicString& word) {
        setFileInd s;
        for(const auto& z:index.freqDictionary.at(word))
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

        setFileInd unionSet;

        set_intersection(result.begin(),result.end(),
                         first.begin(),first.end(),
                         std::inserter(unionSet, unionSet.end()));

        if(unionSet.empty())
            return {};
        else
            result = unionSet;
    }
}

list<tuple<string, float>> SearchServer::getResReq(basicString& text) const {

    while(index.work)
    {
        cin.clear();
        std::cout << "Update base is running, pls wait!!!" << endl;
        this_thread::sleep_for(std::chrono::seconds(3));
    }

    list<FileResult> Results;
    listAnswer out;
    setWords request = getUniqWords(text);
    FileResult::max = 0;

    for(const auto& z: unionSetFiles(request))
        Results.emplace_back(z,request,index);

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

listAnswers SearchServer::getResReqs(const vector<string>& requests) const {

    listAnswers out;

    for(auto request: requests)
        out.push_back(make_tuple(getResReq(request), request));

    addToLog(to_string(requests.size()) + " requests processed!");

    return out;
}

void SearchServer::updateDocumentBase() {

    time = (perf_timer<chrono::seconds>::duration([this] (){
           this->index.updateDocumentBase(settings.threadCount); })
           ).count();

}

SearchServer::SearchServer(Settings&& _settings) :  time{}, index()  {

    settings = (std::move(_settings));
    trustSettings();

    addToLog("Server " + settings.name + " version " + settings.version + " is running!");

    if(settings.dir.empty())
        index.setDocPaths(settings.files);

    auto periodUpdate = [this]()
    {
        while(true)
        {
            if(!work)
            {
                if(!settings.dir.empty())
                    index.setDocPaths(getAllFilesFromDir(settings.dir));

                addToLog("Index database update started!");

                this->updateDocumentBase();
                time = getTimeOfUpdate();

                addToLog("Index database update completed! "+ to_string(index.docPaths.size()) + " files, " + to_string(index.freqDictionary.size()) + " uniq words in dictionary. " + "Time of update " + to_string(time) + " seconds.");
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
            if(!index.work && !index.freqDictionary.empty() && checkHash())
            {
                work = true;
                try
                {
                    ConverterJSON::putAnswers(this->getResReqs(ConverterJSON::getRequests()));
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
        hash = 0;

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
    time_t now = std::time(0);
    strftime( dataTime, sizeof(dataTime),"%H:%M:%S %Y-%m-%d", localtime(&now));

    std::lock_guard<std::mutex> myLock(logMutex);
    logFile.open("log.ini", ios::app);
    logFile << "[" << dataTime << "] " << s << endl;
    logFile.close();
}





SearchServer::FileResult::FileResult(size_t _fileInd, const setWords &request, const InvertedIndex& index) : fileInd(_fileInd)
{
    filePath = index.docPaths.at(fileInd);
    for(const auto& word:request)
        sum += index.freqDictionary.at(word).at(fileInd);
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
