#pragma once
#include <boost/asio.hpp>
#include <iostream>

namespace search_server
{
    class SearchServer;
}

namespace asio_server
{
    using namespace boost::asio::ip;
    using namespace std;
    /**  Класс session, asio_server предстваляют собой доработанные под нужды проекта аналогичные классы из примера
     * асинхронного эхо среввера, взятого из документации boost::asio на сайте библиотеки boost.
     * Сервер может поддерживать одновременно несколько соеденений (session), выполняя квази одновременно (асинхронно)
     * команды от нескольких клиентов. Как уже сказанно сервер - асинхронный, вся работа по поддержке соединений идет в
     * одном потоке.
     *
     * Сервер поддерживает команды:
     * 1. Поисковый запрос (SOLOREQUEST);
     * 2. Запрос текста файла (FILETEXT);
     * 3. Выполнение нескольких запросов из json файла (JSONREGUEST), ответ также предствляет собой json файл.
     *
     * Выполнение любой команды начинается с чтения хедера - сервер читает из сокета хедер состоящий из кода команды и обьема передаваемых данных,
     * если хедер имеет верный формат, то далее считываеются данные в обьеме указанном в хедере, и вы полняется сама команда. Размер хедера строго задан.
     * При отправке ответа в сокет сначала записывается загловок с кодом команды и обьемом информации содержащей результат выполнения команды.
     * Если заголовок имеет неверный формат, то сервер отправляет клиенту хедер с кодом команды ошибка (SOMEERROR) и разрывает содинение
     * (некая защита от случайно прилитевших на порт пакетов).
     * Обьем информации передаваемой в запросе к серверу не должен превышать @param max_length иначе сервер не выполнит команду и сообщит об ошибке.
     * */

    class session : public std::enable_shared_from_this<session>
    {

        enum { max_length = 1024 };
        enum class COMMAND : uint_fast64_t
        {
            SOLOREQUEST = 1,
            FILETEXT,
            JSONREGUEST,
            SOMEERROR
        };
        struct header
        {
            uint_fast64_t size{0};
            COMMAND command{0};
        };

        header header_{};
        tcp::socket socket_;
        char data_[max_length];

        void readSocket();
        void commandExec();
        void writeHeader();
        std::string getAnswer(std::string& request);
        std::string getFile(std::string& request);
        void writeToSocket(const std::string& str);
        std::string getRemoteIP();
        std::string getTextCommand(COMMAND command);
        bool trustCommand();

    public:

        void start();

        session(tcp::socket socket) : socket_(std::move(socket)){};
        ~session();
    };

    class AsioServer
    {
    public:
        AsioServer(boost::asio::io_context& io_context, short port);
    private:
        void do_accept();
        tcp::acceptor acceptor_;
    };
}

