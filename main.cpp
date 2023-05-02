#include <iostream>
#include "SearchServer.h"
#include "ConverterJSON.h"


int main() {

    using namespace std;
    try
    {
        using namespace search_server;
        SearchServer::getInstance();

        string command;

        while(true)
        {
            cout << "---Search Engine is started!---" << endl;
            cout << "----Search Engine work now!----" << endl;
            cout << "------------- Menu ------------" << endl;
            cout << "'1' for enter the request;" << endl;
            cout << "'2' for show server info;" << endl;
            cout << "'3' for exit." << endl;
            cout << "-------------------------------" << endl;
            cout << "Enter the command:";

        cin >> command;
            cin.clear();
            system("cls");

            if(command == "1")
            {
                string request;
                cin.ignore();
                std::cout << "Enter request:";
                std::getline(std::cin, request);

                list<pair<string, float>> results = SearchServer::getInstance().getAnswer(request);

                std::cout << "--- Result: " << results.size() << " ---" << endl;

                for(const auto& result: results)
                    std::cout << result.first << " \trelativeIndex:" << result.second << endl;




                std::cout << "---End---" << endl;
            }
            else if(command == "2")
                SearchServer::getInstance().showSettings();
            else if(command == "3")
                break;
            else
                cout << "Unknown command" << endl;

            system("pause");
            system("cls");
        }
    }
    catch(ConverterJSON::myExp& e)
    {
        e.show();
        system("pause");
    }
    catch(search_server::SearchServer::myExp& e)
    {
        e.show();
        system("pause");
    }

    system("cls");
    std::cout << "---Bye, bye!---" << endl;
    system("pause");

    return 0;
}

