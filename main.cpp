#include <iostream>
#include "SearchServer.h"
#include "InvertedIndex.h"
#include "ConverterJSON.h"

int main() {

    try
    {
        SearchServer myServer(ConverterJSON::getSettings());

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
                basicString request;
                cin.ignore();
                std::cout << "Enter request:";
                std::getline(std::cin, request);

                list<tuple<string, float>> results = myServer.getAnswer(request);

                std::cout << "--- Result: " << results.size() << " ---" << endl;

                for(const auto& result: results)
                    std::cout << get<0>(result) << " \trelativeIndex:" << get<1>(result) << endl;

                std::cout << "---End---" << endl;
            }
            else if(command == "2")
                myServer.showSettings();
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
        ConverterJSON::myExp::show();
        system("pause");
    }
    catch(SearchServer::myExp& e)
    {
        e.show();
        system("pause");
    }

    system("cls");
    std::cout << "---Bye, bye!---" << endl;
    system("pause");

    return 0;
}
