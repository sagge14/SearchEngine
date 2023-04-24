//
// Created by user on 19.02.2023.
//
#include <iostream>
#include "gtest/gtest.h"
#include "InvertedIndex.h"
#include "SearchServer.h"

typedef std::list<std::pair<std::string, float>> listAnswer;

/** Перед запуском тестов необходимо выбрать конфигурацию 'TEST'
  * */

using namespace std;

void TestInvertedIndexFunctionality(
        const vector<string>& docs,
        const vector<string>& requests,
        const vector<inverted_index::mapEntry>& expected) {

    using namespace inverted_index;

    vector<inverted_index::mapEntry> result;
    InvertedIndex::getInstance().updateDocumentBase(docs);

    for(const auto& request : requests) {
        inverted_index::mapEntry word_count = InvertedIndex::getInstance().getWordCount(request);
        result.push_back(word_count);
    }
    ASSERT_EQ(result, expected);
}
TEST(TestInvertedIndexFunctionality, TestBasic1) {
    const vector<string> docs = {
            "london is !the capital of  great britain",
            "big ben is the nickname for !the! Great bell the? of the striking clock"
    };
    const vector<string> requests = {"london", "the"};
    const vector<inverted_index::mapEntry> expected = {
            {
                    {0, 1}
            }, {{
                     0, 1}, {1, 4}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseInvertedIndex, TestBasic2) {
    const vector<string> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const vector<string> requests = {"milk", "water", "cappuccino"};
    const vector<inverted_index::mapEntry> expected = {
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
    const vector<string> docs = {
            "a b c d e f g h i j k l",
            "statement"
    };
    const vector<string> requests = {"m", "statement"};
    const vector<inverted_index::mapEntry> expected = {
            {
            }, {
                    {1, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseSearchServer, TestTop5) {
    const vector<string> docs = {
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
    search_server::SearchServer srv(docs, false);
    listAnswer result = srv.getAnswer(request);
    ASSERT_EQ(result, expected);
}
TEST(TestCaseSearchServer, TestTop5_MissingWord) {
    const vector<string> docs = {
            "Moscow is my favorite city, also Moscow is the capital of Russia",
            "berlin is the capital of germany",
            "I was born in Moscow",
            "I from Russia",
            "A capital is main city in any country",
            "moscow is the capital of russia!",
    };
    string request = {"moscow is the capital of russia MissingWord"};
    const listAnswer expected = {

                    {"0", 1},
                    {"5", 0.75},
                    {"1", 0.5},
                    {"4", 0.25},
                    {"2", 0.125}
    };
    search_server::SearchServer srv(docs, false);
    listAnswer result = srv.getAnswer(request);
    ASSERT_EQ(result, expected);
}
TEST(TestCaseSearchServer, TestTop5_Extact_Search) {
    const vector<string> docs = {
            "Moscow is my favorite city, also Moscow is the capital of Russia",
            "berlin is the capital of germany",
            "I was born in Moscow",
            "I from Russia",
            "A capital is main city in any country",
            "moscow is the capital of russia",
    };
    string request = {"moscow is the capital of Russia"};
    const listAnswer expected = {
            {
                    {"0", 1},
                    {"5", 0.75}
            }
    };
    search_server::SearchServer srv(docs, true);
    listAnswer result = srv.getAnswer(request);
    ASSERT_EQ(result, expected);
}
TEST(TestCaseSearchServer, TestTop5_Extact_Search_MissingWord) {
    const vector<string> docs = {
            "Moscow is my favorite city, also Moscow is the capital of Russia",
            "berlin is the capital of germany",
            "I was born in Moscow",
            "I from Russia",
            "A capital is main city in any country",
            "moscow is the capital of russia",
    };
    string request = {"moscow is the capital of Russia MissingWord"};

    const listAnswer expected = {};

    search_server::SearchServer srv(docs, true);
    listAnswer result = srv.getAnswer(request);
    ASSERT_EQ(result, expected);
}
int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}