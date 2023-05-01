//
// Created by Sg on 07.03.2023.
//

#include "AsioServer.h"
#include "SearchServer.h"
#include <iostream>

using boost::asio::ip::tcp;

void asio_server::session::start() {
    readSocket();
}

void asio_server::session::readSocket() {

    using namespace search_server;
    auto self(shared_from_this());

    //std::cout << "Connect\t\t" + getRemoteIP() << std::endl;
    SearchServer::getInstance().addToLog("Connect\t\t" + getRemoteIP());

    socket_.async_read_some(boost::asio::buffer(&header_, sizeof (header)),
                            [this, self](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    //std::cout << "Header read\t" +getRemoteIP() << std::endl;
                                    SearchServer::getInstance().addToLog("Header read\t" + getRemoteIP()) ;

                                    if(trustCommand())
                                    {
                                        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                                                [this, self](boost::system::error_code ec, std::size_t length)
                                                                {
                                                                    if (!ec)
                                                                    {
                                                                       // std::cout << "Received from\t" + getRemoteIP() + "\tcommand '" + getTextCommand(header_.command) << std::endl;
                                                                        SearchServer::getInstance().addToLog("Received from\t" + getRemoteIP() + "\tcommand '" + getTextCommand(header_.command));

                                                                        commandExec();
                                                                    }
                                                                });
                                    }
                                    else
                                    {
                                        header_.command = COMMAND::SOMEERROR;
                                        SearchServer::getInstance().addToLog("Received from\t" + getRemoteIP() + "\tcommand\t'" + getTextCommand(header_.command)+"'");
                                        writeHeader();
                                        //std::cout << "Socket off\t" + getRemoteIP() << std::endl;
                                        SearchServer::getInstance().addToLog("Socket off\t" + getRemoteIP());
                                    }
                                }
                            });

}

void asio_server::session::commandExec() {

    std::string request(data_, header_.size);
    std::string answer;

    switch (header_.command) {
        case COMMAND::SOLOREQUEST:
        {
            answer = getAnswer(request); break;
        }
        case COMMAND::FILETEXT:
        {
            answer = getFile(request); break;
        }
        case COMMAND::JSONREGUEST:
        {
            answer = ConverterJSON::putAnswers(search_server::SearchServer::getInstance().getAllAnswers(ConverterJSON::getRequestsFromString(request))); break;
        }

    }

    header_.size = std::strlen(answer.c_str());
    writeHeader();
    writeToSocket(answer);
}



void asio_server::session::writeHeader() {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(&header_, sizeof(header)),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if(header_.command == COMMAND::SOMEERROR)
                                 {
                                     socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                                     socket_.close(ec);
                                     header_ = header{};
                                 }
                             });
}

std::string asio_server::session::getAnswer(std::string& request) {

    stringstream ss;

    list<pair<string, float>> results = search_server::SearchServer::getInstance().getAnswer(request);

    for(const auto& result: results)
        ss << result.first << " " << result.second << endl;

    return ss.str();
}

std::string asio_server::session::getFile(string &request) {

    fstream file(request);
    if(file.is_open())
    {
        std::string text((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
        return std::move(text);
    }
        return "";
}

std::string asio_server::session::getRemoteIP() {

    boost::asio::ip::tcp::endpoint remote_ep = socket_.remote_endpoint();
    boost::asio::ip::address remote_ad = remote_ep.address();

    return remote_ad.to_string();
}

void asio_server::session::writeToSocket(const std::string& str) {

    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer(str, std::strlen(str.c_str())),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if (!ec)
                                 {

                                     search_server::SearchServer::getInstance().addToLog("Request to\t"+ getRemoteIP() + "\tcommand\t'" + getTextCommand(header_.command) +"'\tsize " +
                                        std::to_string(header_.size) + "\tbytes");

                                     readSocket();
                                 }
                             });
}

bool asio_server::session::trustCommand() {

    bool trust;

    switch(header_.command) {
        case COMMAND::SOLOREQUEST:
        case COMMAND::FILETEXT:
        case COMMAND::JSONREGUEST:
            trust = true;
    }

    if(header_.command == COMMAND::SOMEERROR || header_.size > max_length)
        trust = false;

    return trust;
}

std::string asio_server::session::getTextCommand(COMMAND command) {
    switch (command) {
        case COMMAND::JSONREGUEST:
            return "JSON request";
        case COMMAND::FILETEXT:
            return "FILE text";
        case COMMAND::SOLOREQUEST:
            return "SINGLE request";
        case COMMAND::SOMEERROR:
            return "ERROR :(";
    }
    return "";
}

asio_server::session::~session() {

    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

void asio_server::AsioServer::do_accept() {

    acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                    std::make_shared<session>(std::move(socket))->start();

                do_accept();
            });

}

asio_server::AsioServer::AsioServer(boost::asio::io_context &io_context, short port)
: acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
    do_accept();
}


