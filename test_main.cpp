//
// Created by user on 19.02.2023.
//
#include <iostream>
#include "gtest/gtest.h"
#include "InvertedIndex.h"
#include "SearchServer.h"

typedef std::list<std::tuple<std::string, float>> listAnswer;

/** Перед запуском тестов необходимо в файле "Settings.h"
  * установить #define TEST_MODE true
  * */

using namespace std;

void TestInvertedIndexFunctionality(
        const vector<basicString>& docs,
        const vector<basicString>& requests,
        const vector<mapEntry>& expected
) {
    vector<mapEntry> result;
    InvertedIndex idx(docs);
    idx.updateDocumentBase();
    for(const auto& request : requests) {
        mapEntry word_count = idx.getWordCount(request);
        result.push_back(word_count);
    }
    ASSERT_EQ(result, expected);
}
TEST(TestInvertedIndexFunctionality, test3) {
    const vector<basicString> docs = {
            "london is !the capital of  great britain",
            "big ben is the nickname for !the! Great bell the? of the striking clock"
    };
    const vector<basicString> requests = {"london", "the"};
    const vector<mapEntry> expected = {
            {
                    {0, 1}
            }, {{
                     0, 1}, {1, 4}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseInvertedIndex, TestBasic2) {
    const vector<basicString> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const vector<basicString> requests = {"milk", "water", "cappuccino"};
    const vector<mapEntry> expected = {
            {
                    {0, 4}, {1, 1}, {2, 5}
            }, {
                    {0, 3}, {1, 2}, {2, 5}
            }, {
                    {3, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord) {
    const vector<basicString> docs = {
            "a b c d e f g h i j k l",
            "statement"
    };
    const vector<basicString> requests = {"m", "statement"};
    const vector<mapEntry> expected = {
            {
            }, {
                    {1, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseSearchServer, TestTop5) {
    const vector<basicString> docs = {
            "london is the capital of great britain",
            "paris is the capital of france",
            "berlin is the capital of germany",
            "rome is the capital of italy",
            "madrid is the capital of spain",
            "lisboa is the capital of portugal",
            "bern is the capital of switzerland",
            "moscow is the capital of russia",
            "kiev is the capital of ukraine",
            "minsk is the capital of belarus",
            "astana is the capital of kazakhstan",
            "beijing is the capital of china",
            "tokyo is the capital of japan",
            "bangkok is the capital of thailand",
            "welcome to moscow the capital of russia the third rome",
            "amsterdam is the capital of netherlands",
            "helsinki is the capital of finland",
            "oslo is the capital of norway",
            "stockholm is the capital of sweden",
            "riga is the capital of latvia",
            "tallinn is the capital of estonia",
            "warsaw is the capital of poland",
    };
    string request = {"moscow is the capital of russia"};
    const listAnswer expected = {
            {
                    {"7", 1},
                    {"14", 1},
                    {"0", 0.666666687},
                    {"1", 0.666666687},
                    {"2", 0.666666687}
              }
    };
    SearchServer srv(docs);
    listAnswer result = srv.getAnswer(request);
    ASSERT_EQ(result, expected);
}
int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}