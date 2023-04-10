#pragma once
#include <cstdlib>
#include "stdint.h"
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

class SearchServer;

using boost::asio::ip::tcp;



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
    SearchServer* searchServer_;
    session(tcp::socket socket, SearchServer* searchServer) : socket_(std::move(socket)), searchServer_{searchServer}{};

};

class AsioServer
{
public:
    AsioServer(boost::asio::io_context& io_context, short port, SearchServer* searchServer);

private:
    void do_accept();
    SearchServer* searchServer_;
    tcp::acceptor acceptor_;
};