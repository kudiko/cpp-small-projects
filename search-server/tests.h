#pragma once



class Tests
{
    public:
    void TestSearchServer();
    private:
    static constexpr double MAX_WORD_FREQ_DIFFERENCE = 1e-6;
    static void TestDocumentAddition();
    static void TestExcludeStopWordsFromAddedDocumentContent();
    static void TestExcludesDocumentsWithMinusWords();
    static void TestDocumentMatching();
    static void TestCorrectRelevanceCalculation();
    static void TestCorrectRelevanceSort();
    static void TestRatingCalculation();
    static void TestSearchDocumentsWithSelectedStatus();
    static void TestFilterDocumentsUsingPredicate();
    static void TestSprint6Functional();
};


