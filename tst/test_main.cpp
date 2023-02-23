//
// Created by user on 19.02.2023.
//
#include <iostream>
#include "gtest/gtest.h"
#include "InvertedIndex.h"


using namespace std;

typedef string basicString;
typedef map<size_t,size_t> mapEntry;


void TestInvertedIndexFunctionality(
        const vector<string>& docs,
        const vector<string>& requests,
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
    const vector<string> docs = {
            "london is !the capital of  great britain",
            "big ben is the nickname for !the! Great bell the? of the striking clock"
    };
    const vector<string> requests = {"london", "the"};
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
    const vector<string> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const vector<string> requests = {"milk", "water", "cappuccino"};
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
    const vector<string> docs = {
            "a b c d e f g h i j k l",
            "statement"
    };
    const vector<string> requests = {"m", "statement"};
    const vector<mapEntry> expected = {
            {
            }, {
                    {1, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}