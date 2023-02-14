#include <iostream>
#include "SearchServer.h"
#include "InvertedIndex.h"
#include "ConverterJSON.h"



#include <locale>
int main() {

    try
    {
        SearchServer myServer(ConverterJSON::getSettings());
        basicString request;

        while(true)
        {
            std::cout << "---Search---" << endl;
            std::cout << "Enter request:";
            std::getline(std::cin, request);
            cin.clear();

            if(request == "-1")
                break;

            list<tuple<string, float>> results = myServer.getResReq(request);

            std::cout << "--- Result: " << results.size() << " ---" << endl;

            for(const auto& result: results)
                std::cout << get<0>(result) << " \tRelativeIndex:" << get<1>(result) << endl;

            std::cout << "---End---" << endl;
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

    return 0;
}
