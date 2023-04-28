#include "tests.h"

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>

#include "search_server.h"
#include "document.h"
#include "remove_duplicates.h"
using namespace std::literals::string_literals;



template <typename T> std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) 
{
    out << "["s;
    bool is_first = true;
    for (const auto& element : v)
    {
        if(!is_first)
        {
            out << ", "s;
        }
        out << element;
        is_first = false;
    }
    out << "]"s;
    return out;
}
template <typename T, typename U> std::ostream& operator<< (std::ostream& out, const std::map<T, U>& m) 
{
    out << "{"s;
    bool is_first = true;
    for (const auto& [key, value] : m)
    {
        if(!is_first)
        {
            out << ", "s;
        }
        out << key << ": "s << value;
        is_first = false;
    }
    out << "}"s;
    return out;
}
template <typename T> std::ostream& operator<< (std::ostream& out, const std::set<T>& s) 
{
    out << "{"s;
    bool is_first = true;
    for (const auto& element : s)
    {
        if(!is_first)
        {
            out << ", "s;
        }
        out << element;
        is_first = false;
    }
    out << "}"s;
    return out;
}

std::ostream& operator<< (std::ostream& out, const DocumentStatus status)
{
    switch (status)
    {
    case DocumentStatus::ACTUAL :
        out << "DocumentStatus::ACTUAL";
        break;
    case DocumentStatus::BANNED :
        out << "DocumentStatus::BANNED";
        break;
    case DocumentStatus::IRRELEVANT :
        out << "DocumentStatus::IRRELEVANT";
        break;
    case DocumentStatus::REMOVED :
        out << "DocumentStatus::REMOVED";
        break;
    }
    return out;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) 
{
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
                const std::string& hint) 
{
    if (!value) {
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))


template <typename T>
void RunTestImpl(T test_func, const std::string& test_func_str) 
{
    test_func();
    std::cerr << test_func_str << " OK" << std::endl;
}

#define RUN_TEST(tested_func) RunTestImpl((tested_func), #tested_func) 

// -------- Начало модульных тестов поисковой системы ----------

// тест добавления документов
void Tests::TestDocumentAddition()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};

    const int second_doc_id = 1;
    const std::string second_content = "orange cat near the library"s;
    const std::vector<int> second_ratings = {4, 5, 6};
    // test of known documents count
    {
        SearchServer server(""s);
        ASSERT_EQUAL_HINT(server.GetDocumentCount(), 0, "There should be 0 known docs if we didn't add anything"s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        ASSERT_EQUAL_HINT(server.GetDocumentCount(), 1, "There should be 1 known doc if we added it"s);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::ACTUAL, second_ratings);
        ASSERT_EQUAL_HINT(server.GetDocumentCount(), 2, "There should be 2 known docs if we added them"s);
    }
    // one document test
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Adding doc should increase the size of found docs for a query with a word from a doc"s);
        ASSERT_EQUAL_HINT(found_docs[0].id, first_doc_id, "The only found document should have the same id as the added one"s);
        found_docs = server.FindTopDocuments(""s);
        ASSERT_HINT(found_docs.empty(), "Search with an empty string query shouldn't yield anything"s);
        found_docs = server.FindTopDocuments("word"s);
        ASSERT_HINT(found_docs.empty(), "Search with a word, which is not in the document, shouldn't yield anything"s);
    }   
    // two document test
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::ACTUAL, second_ratings);
        auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 2u, "query with word, which is in both docs, should return both of them"s);
        ASSERT_EQUAL_HINT(found_docs[1].id, first_doc_id, "First document should be at the back of the vector because relevance of both is 0, and second doc has higher rating"s);
        ASSERT_EQUAL_HINT(found_docs[0].id, second_doc_id, "Second document should be at the front of the vector (see prev. line)"s);
        found_docs = server.FindTopDocuments(""s);
        ASSERT_HINT(found_docs.empty(), "Search with an empty string query still shouldn't yield anything"s);
        found_docs = server.FindTopDocuments("near"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u, "query with a word, which is only in one of docs, should return one doc"s);
        ASSERT_EQUAL_HINT(found_docs[0].id, second_doc_id, "query with a word, which is only in one of docs, should return that docs' id");
    }
}
// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void Tests::TestExcludeStopWordsFromAddedDocumentContent() 
{
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};

    // сделаем пару слов из документа стоп-словами и проверим, что по ним уже документ не находится. Также проверим, что по остальным продолжает находиться
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
        ASSERT_HINT(server.FindTopDocuments("the"s).empty(), "Stop words must be excluded from documents"s);
        ASSERT_EQUAL_HINT(server.FindTopDocuments("cat"s).size(), 1u, "Words which are not stop, should still find the document"s);
        ASSERT_EQUAL_HINT(server.FindTopDocuments("city"s).size(), 1u, "Words which are not stop, should still find the document"s);
    }
}
// тест правильной логики работы с минус-словами
void Tests::TestExcludesDocumentsWithMinusWords()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};

    const int second_doc_id = 1;
    const std::string second_content = "orange cat near the library"s;
    const std::vector<int> second_ratings = {4, 5, 6};
    //one document one minus word
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        const auto found_docs = server.FindTopDocuments("cat -city"s);
        ASSERT_HINT(found_docs.empty(), "query including minus word, which is in doc, shouldn't return anything"s);
    }
    //two document two excluding minus words
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::ACTUAL, second_ratings);
        const auto found_docs = server.FindTopDocuments("cat -in -near"s);
        ASSERT_HINT(found_docs.empty(), "query including two minus words, which are in both docs shouldn't return anything"s);
    }
    //two document one excluded
    {   
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::ACTUAL, second_ratings);
        const auto found_docs = server.FindTopDocuments("cat -city"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u, "query including one minus word, which is in one doc should return the other"s);
        ASSERT_EQUAL_HINT(found_docs[0].id, second_doc_id, "Id of doc without minus words should be in the results"s);
    }
}
// тест соответствия документов поисковому запросу
void Tests::TestDocumentMatching()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};
    // wrong word query
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        const auto [matched_words, document_status] = server.MatchDocument("word"s, first_doc_id);
        ASSERT_HINT(matched_words.empty(), "Matching of a query with a word, which is not in a doc, shouldn't return any matching words"s);
        ASSERT_EQUAL_HINT(document_status, DocumentStatus::ACTUAL, "Doc status shouldn't change after matching"s);
    }
    // two known words query
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        const auto [matched_words, document_status] = server.MatchDocument("cat city"s, first_doc_id);
        std::vector<std::string> expected_matched_words {"cat"s, "city"s};
        ASSERT_EQUAL_HINT(matched_words, expected_matched_words, 
        "Matching of a query with two words from a doc should return them both in an order like in their parent doc"s);
        ASSERT_EQUAL_HINT(document_status, DocumentStatus::ACTUAL, "Doc status shouldn't change after matching"s);
    }
    // two known words query with stop words
    {
        SearchServer server("in the"s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        const auto [matched_words, document_status] = server.MatchDocument("cat in the city"s, first_doc_id);
        std::vector<std::string> expected_matched_words {"cat"s, "city"s};
        ASSERT_EQUAL_HINT(matched_words, expected_matched_words, 
        "Matching of 2 stop and 2 plus word query should return plus words in order like in their parent doc"s);
        ASSERT_EQUAL_HINT(document_status, DocumentStatus::ACTUAL, "Doc status shouldn't change after matching"s);
    }
    // query with a minus word
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        const auto [matched_words, document_status] = server.MatchDocument("cat -city"s, first_doc_id);
        ASSERT_HINT(matched_words.empty(), "Matching of a query including minus word shouldn't return anything"s);
        ASSERT_EQUAL_HINT(document_status, DocumentStatus::ACTUAL, "Doc status shouldn't change after matching"s);
    }
}
// тест корректного вычисления релевантности
void Tests::TestCorrectRelevanceCalculation()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};

    const int second_doc_id = 1;
    const std::string second_content = "orange cat near the library cat"s;
    const std::vector<int> second_ratings = {4, 5, 6};
    // one doc with known word query
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL_HINT(result.size(), 1u, "We should find a doc containing known word"s);
        constexpr double ref_IDF = 0;
        constexpr double ref_TF = 1.0 / 4;  
        ASSERT_HINT(std::abs(result.back().relevance - ref_IDF * ref_TF) < MAX_RELEVANCE_DIFFERENCE, "Calculated relevance should match the reference one"s);
    }
    // two docs with a two word query, first word is in both docs, second is only in one
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::ACTUAL, second_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat city");
        ASSERT_EQUAL_HINT(result.size(), 2u, "We should find both docs as they contain words from a query"s);
        const double ref_IDF_cat = 0;
        const double ref_IDF_city = std::log(2.0 / 1);

        constexpr double ref_TF1_cat = 1.0 / 4;  
        constexpr double ref_TF1_city = 1.0 / 4;
        constexpr double ref_TF2_cat = 2.0 / 6;  
        constexpr double ref_TF2_city = 0;
        ASSERT_HINT(std::abs(result[0].relevance - (ref_IDF_cat * ref_TF1_cat + ref_IDF_city * ref_TF1_city)) < MAX_RELEVANCE_DIFFERENCE, "Calculated relevance should match the reference one"s);
        ASSERT_HINT(std::abs(result[1].relevance - (ref_IDF_cat * ref_TF2_cat + ref_IDF_city * ref_TF2_city)) < MAX_RELEVANCE_DIFFERENCE, "Calculated relevance should match the reference one"s);
    }
}
// тест правильной сортировки по релевантности
void Tests::TestCorrectRelevanceSort()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};

    const int second_doc_id = 1;
    const std::string second_content = "cat cat cat cat"s;
    const std::vector<int> second_ratings = {4, 5, 6};
    
    const int third_doc_id = 2;
    const std::string third_content = "black dog train station"s;
    const std::vector<int> third_ratings = {7, 8, 9};

    const int fourth_doc_id = 3;
    const std::string fourth_content = "black cat train cat"s;
    const std::vector<int> fourth_ratings = {2, 2, 2};
    
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::ACTUAL, second_ratings);
        server.AddDocument(third_doc_id, third_content, DocumentStatus::ACTUAL, third_ratings);
        server.AddDocument(fourth_doc_id, fourth_content, DocumentStatus::ACTUAL, fourth_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat"s);
        ASSERT_HINT(std::is_sorted(cbegin(result), cend(result), [](const Document& lhs, const Document& rhs) { return lhs.relevance > rhs.relevance; }), 
        "Relevances should be sorted in descending order"s);
    }
}
// тест правильного подсчета рейтинга
void Tests::TestRatingCalculation()
{
    const int third_doc_id = 2;
    const std::string third_content = "black cat found near train station"s;
    const std::vector<int> third_ratings = {1, 1, 3};

    {
        SearchServer server(""s);
        server.AddDocument(third_doc_id, third_content, DocumentStatus::ACTUAL, third_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat near library");
        const int ref_third_rating = std::accumulate(begin(third_ratings), end(third_ratings), 0) / static_cast<int>(third_ratings.size());
        ASSERT_EQUAL_HINT(result.size(), 1u, "We should get only one element in the output vector"s);
        ASSERT_EQUAL_HINT(result[0].rating, ref_third_rating, "Rating should be calculated as mean"s);
    }
}
// тест поиска документов по выбранному статусу
void Tests::TestSearchDocumentsWithSelectedStatus()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};

    const int second_doc_id = 1;
    const std::string second_content = first_content;
    const std::vector<int> second_ratings = {4, 5, 6};
    
    const int third_doc_id = 2;
    const std::string third_content = first_content;
    const std::vector<int> third_ratings = {1, 1, 3};

    const int fourth_doc_id = 3;
    const std::string fourth_content = first_content;
    const std::vector<int> fourth_ratings = {2, 2, 2};
    // trying to find doc with a selected status
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::BANNED, second_ratings);
        server.AddDocument(third_doc_id, third_content, DocumentStatus::REMOVED, third_ratings);
        server.AddDocument(fourth_doc_id, fourth_content, DocumentStatus::IRRELEVANT, fourth_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat", DocumentStatus::BANNED);
        ASSERT_EQUAL_HINT(result.size(), 1u, "We should find one doc"s);
        ASSERT_EQUAL_HINT(result[0].id, second_doc_id, "We should find only the doc with a status we asked for"s);
    }
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::REMOVED, first_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat", DocumentStatus::ACTUAL);
        ASSERT_HINT(result.empty(), "We shouldn't find a doc with a status, which is different from the status of an added doc"s);
    }
}
// тест фильтрации документов с помощью предиката
void Tests::TestFilterDocumentsUsingPredicate()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};

    const int second_doc_id = 1;
    const std::string second_content = "cat in the city"s;
    const std::vector<int> second_ratings = {4, 5, 6};
    
    const int third_doc_id = 2;
    const std::string third_content = "cat in the city"s;
    const std::vector<int> third_ratings = {3, 3, 3};
    // trying to use status predicate
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::BANNED, second_ratings);
        server.AddDocument(third_doc_id, third_content, DocumentStatus::REMOVED, third_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat", [](int document_id, DocumentStatus status, int rating) 
        { return status == DocumentStatus::BANNED; });
        ASSERT_EQUAL_HINT(result.size(), 1u, "Such a predicate should get us only BANNED status doc"s);
        ASSERT_EQUAL_HINT(result.back().id, second_doc_id, "We should only get doc with a BANNED status"s);
    }
    // trying to use even id predicate
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.AddDocument(second_doc_id, second_content, DocumentStatus::BANNED, second_ratings);
        server.AddDocument(third_doc_id, third_content, DocumentStatus::REMOVED, third_ratings);
        const std::vector<Document> result = server.FindTopDocuments("cat", [](int document_id, DocumentStatus status, int rating) 
        { return document_id % 2 == 0; });
        ASSERT_EQUAL_HINT(result.size(), 2u, "We should get only two even docs in the output"s);
        ASSERT_EQUAL_HINT(result.back().id, first_doc_id, "Returned docs should be in a correct order"s);
        ASSERT_EQUAL_HINT(result.front().id, third_doc_id, "Returned docs should be in a correct order"s);
        // relevance of them both is 0, but rating of 3rd is 3 and rating of 1st is 2
    }
}

void Tests::TestSprint6Functional()
{
    const int first_doc_id = 42;
    const std::string first_content = "cat in the city"s;
    const std::vector<int> first_ratings = {1, 2, 3};

    const int second_doc_id = 1;
    const std::string second_content = "orange cat near the library"s;
    const std::vector<int> second_ratings = {4, 5, 6};
    // SearchServer iterator test
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        for (const int doc_id : server)
        {
            ASSERT_EQUAL_HINT(doc_id, first_doc_id, "We should get only one existing doc id"s);
        }
        server.AddDocument(second_doc_id, second_content, DocumentStatus::ACTUAL, second_ratings);
        std::set<int> expected_ids{first_doc_id , second_doc_id};
        std::set<int> written_ids;
        for (auto it = server.begin(); it != server.end(); ++it)
        {
            written_ids.insert(*it);
        }
        ASSERT_EQUAL_HINT(written_ids, expected_ids, "Vectors of expected and written ids should match"s);
    }
    // GetWordFrequencies test
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);

        auto answer = server.GetWordFrequencies(second_doc_id);
        ASSERT_HINT(answer.empty(), "Answer map should be empty for non-existing document"s);

        const std::map<std::string, double> expected_answer{{"cat"s, 0.25}, {"in"s, 0.25}, {"the"s, 0.25}, {"city"s, 0.25}};
        answer = server.GetWordFrequencies(first_doc_id);
        ASSERT_EQUAL_HINT(expected_answer.size(), answer.size(), "Maps' sizes should match"s);
        for (const auto& [word, freq] : answer)
        {
            ASSERT_HINT(expected_answer.count(word), "Word should be in expected answer"s);
            ASSERT_HINT(std::abs(expected_answer.at(word) - freq) < Tests::MAX_WORD_FREQ_DIFFERENCE, "Word  freqs should match"s);
        }
    }
    // RemoveDocument test
    {
        SearchServer server(""s);
        server.AddDocument(first_doc_id, first_content, DocumentStatus::ACTUAL, first_ratings);
        server.RemoveDocument(first_doc_id);
        ASSERT_HINT(
        (server.word_to_document_frequency_.count("cat"s) == 0) &&
        (server.document_data_.count(first_doc_id) == 0) &&
        (server.added_documents_.count(first_doc_id) == 0) &&
        (server.doc_id_to_word_frequency_.count(first_doc_id) == 0) &&
        (server.begin() == server.end()), 
        "Document should be erased from all structures"s
        );
    }
    // RemoveDuplicatesTest
    {
        SearchServer search_server("and with"s);
        search_server.AddDocument( 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
        search_server.AddDocument( 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        // дубликат документа 2, будет удалён
        search_server.AddDocument( 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        // отличие только в стоп-словах, считаем дубликатом
        search_server.AddDocument( 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        // множество слов такое же, считаем дубликатом документа 1
        search_server.AddDocument( 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
        // добавились новые слова, дубликатом не является
        search_server.AddDocument( 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
        // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
        search_server.AddDocument( 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, {1, 2});
        // есть не все слова, не является дубликатом
        search_server.AddDocument( 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, {1, 2});
        // слова из разных документов, не является дубликатом
        search_server.AddDocument( 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        ASSERT_EQUAL_HINT(search_server.GetDocumentCount(), 9, "We should have all the added documents"s);
        RemoveDuplicates(search_server);
        ASSERT_EQUAL_HINT(search_server.GetDocumentCount(), 5, "All the duplicates should be deleted"s);
    }
}   

// Функция TestSearchServer является точкой входа для запуска тестов
void Tests::TestSearchServer() {
    RUN_TEST(Tests::TestDocumentAddition);
    RUN_TEST(Tests::TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(Tests::TestExcludesDocumentsWithMinusWords);
    RUN_TEST(Tests::TestDocumentMatching);
    RUN_TEST(Tests::TestCorrectRelevanceCalculation);
    RUN_TEST(Tests::TestCorrectRelevanceSort);
    RUN_TEST(Tests::TestRatingCalculation);
    RUN_TEST(Tests::TestSearchDocumentsWithSelectedStatus);
    RUN_TEST(Tests::TestFilterDocumentsUsingPredicate);
    RUN_TEST(Tests::TestSprint6Functional);
}
