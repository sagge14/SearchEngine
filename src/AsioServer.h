#pragma once
#include <boost/asio.hpp>

namespace search_server
{
    class SearchServer;
}

namespace asio_server
{
    using namespace boost::asio::ip;
    using namespace std;

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
        search_server::SearchServer* searchServer_;
        session(tcp::socket socket, search_server::SearchServer* searchServer) : socket_(std::move(socket)), searchServer_{searchServer}{};

    };

    class AsioServer
    {
    public:
        AsioServer(boost::asio::io_context& io_context, short port, search_server::SearchServer* searchServer);

    private:
        void do_accept();
        search_server::SearchServer* searchServer_;
        tcp::acceptor acceptor_;
    };
}

