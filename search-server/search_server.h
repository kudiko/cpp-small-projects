#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdexcept>

#include "string_processing.h"
#include "document.h"
#include "tests.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;
constexpr double MAX_RELEVANCE_DIFFERENCE = 1e-6;

using namespace std::literals::string_literals;

class SearchServer
{
public:
    explicit SearchServer(const std::string& text);

    template <typename StringCollection>
    SearchServer(const StringCollection &stop_words_init);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    // перегруженная функция для обработки аргументов только из строки
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const; 

    // перегруженная функция для обработки аргументов только из строки+статуса
    std::vector<Document> FindTopDocuments(const std::string& raw_query, const DocumentStatus doc_status) const; 

    // вторым аргументом принимаем лямбду-фильтр документов
    template <typename Filter>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, Filter filtering_predicat) const; 

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

    int GetDocumentCount() const;

    std::set<int>::const_iterator begin() const;

    std::set<int>::const_iterator end() const;

    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);
private:

    // позволяет тестам смотреть в приватные поля класса
    friend class Tests;

    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    // храним в map для каждого встреченного слова id документов и частоту слова
    std::map<std::string, std::map<int, double>> word_to_document_frequency_; 

    // в множестве храним стоп-слова
    const std::set<std::string> stop_words_;

    // по id документа храним структуру с его рейтингом и статусом
    std::map<int, DocumentData> document_data_;

    // храним id всех добавленных документов
    std::set<int> added_documents_;

    //для метода GetWordFrequencies, хотим чтобы он работал за O(log N), что достигается в мэпе
    std::map<int, std::map<std::string, double>> doc_id_to_word_frequency_;
    
    struct ProcessedQuery
    {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    struct QueryWord
    {
        std::string word;
        bool is_minus_word;
        bool is_stop_word;
    };

    bool IsStopWord(const std::string& word) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    // функция обработки слова (отбрасываем минус если он есть), и постановки флагов минус-слова и стоп-слова
    QueryWord ProcessQueryWord(std::string raw_word) const; 

    //возвращаем множества плюс- и минус- слов
    ProcessedQuery ParseQuery(const std::string& text) const; 

    // ищем все документы, которые содержат слова из запроса
    template <typename FilterFunction>
    std::vector<Document> FindAllDocuments(ProcessedQuery processed_query, FilterFunction filtering_predicat) const; 
    
    double CalculateIDF(const std::string& word) const; // считаем IDF слова 

    static int ComputeAverageRating(const std::vector<int>& ratings);
    
    static bool IsValidWord(const std::string& text);
};

template <typename StringCollection>
SearchServer::SearchServer(const StringCollection &stop_words_init) : stop_words_(MakeUniqueNonEmptyStrings(stop_words_init))
{
    if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord))
    {
        throw std::invalid_argument("There must be no special symbols in a stop word"s);
    }
}

template <typename Filter>
std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query, Filter filtering_predicat) const
{
    const ProcessedQuery query = ParseQuery(raw_query); // query input errors are thrown here
    auto matched_documents = FindAllDocuments(query, filtering_predicat);
    std::sort(matched_documents.begin(), matched_documents.end(),
              [](const Document &lhs, const Document &rhs)
              {
                  // ОСТОРОЖНО! если не дописать здесь std:: перед abs,
                  // вызовется сишная функция abs, работающая только с интами - произойдут неявные преобразования типов, 
                  // и все будет неправильно работать
                  if (std::abs(lhs.relevance - rhs.relevance) < MAX_RELEVANCE_DIFFERENCE)
                  {
                      return lhs.rating > rhs.rating;
                  }
                  else
                  {
                      return lhs.relevance > rhs.relevance;
                  }
              });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
    {
    matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}


// ищем все документы, которые содержат слова из запроса
// и фильтруем результат с помощью фильтрующей лямбда-функции
template <typename FilterFunction>
std::vector<Document> SearchServer::FindAllDocuments(ProcessedQuery processed_query, FilterFunction filtering_predicat) const 
{                                                                                                               
    std::map<int, double> matched_documents;
    for (const auto &plus_word : processed_query.plus_words)
    {
    if (word_to_document_frequency_.count(plus_word))
    {
        double IDF = CalculateIDF(plus_word);
        for (const auto &[document_id, freq] : word_to_document_frequency_.at(plus_word))
        {
            // вызываем фильтрующую лямбда-функцию
            if (filtering_predicat(document_id, document_data_.at(document_id).status, document_data_.at(document_id).rating))
            {
                matched_documents[document_id] += IDF * freq; // считаем релевантность документа
            }
        }
    }
    }
    
    // сначала записываем все документы, содержащие слова, не являющиеся минус- , в результат. 
    // Следующим циклом уже удалим из результата документы, содержащие минус-слова
    for (const auto &minus_word : processed_query.minus_words)
    {
    if (word_to_document_frequency_.count(minus_word))
    {
        for (const auto &[document_id, freq] : word_to_document_frequency_.at(minus_word))
        {
            matched_documents.erase(document_id); // убираем из выдачи документ, содержащий минус-слово
        }
    }
    }
    std::vector<Document> vector_of_matched_documents;
    for (const auto &[document_id, relevance] : matched_documents) // из словаря делаем вектор выдачи
    {
        vector_of_matched_documents.push_back({document_id, relevance, document_data_.at(document_id).rating});
    }
    return vector_of_matched_documents;
}