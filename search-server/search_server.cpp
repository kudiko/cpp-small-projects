#include "search_server.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <numeric>

#include "string_processing.h"
#include "document.h"

using namespace std::literals::string_literals;

SearchServer::SearchServer(const std::string &text) : SearchServer(SplitIntoWords(text)) {}

void SearchServer::AddDocument(int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings)
{
    if (document_id < 0 || document_data_.count(document_id) == 1)
    {
        throw std::invalid_argument("Could not add document with negative or already occupied id"s);
    }
    const std::vector<std::string> words = SplitIntoWordsNoStop(document);
    for (const auto &word : words)
    {
        if (!IsValidWord(word))
        {
            throw std::invalid_argument("There must be no special symbols in a document content"s);
        }

        // записываем относительную частоту слова в документе (TF)
        word_to_document_frequency_[word][document_id] += 1.0 / words.size();
        // тоже записываем TF, но в еще одну удобную структуру для получения частоты слов в док-те по id
        doc_id_to_word_frequency_[document_id][word] += 1.0 / words.size();
    }
    document_data_[document_id] = {ComputeAverageRating(ratings), status};
    added_documents_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query) const
{
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query, const DocumentStatus doc_status) const
{
    return FindTopDocuments(raw_query, [doc_status](int document_id, DocumentStatus status, int rating)
                            { return status == doc_status; });
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string &raw_query, int document_id) const
{
    const ProcessedQuery query = ParseQuery(raw_query); // query input errors are thrown there
    std::vector<std::string> plus_words_in_document;

    // сначала обработаем минус-слова, если найдем минус-слова, то вернем сразу пустой вектор и выйдем из функции
    for (const std::string &minus_word : query.minus_words) 
    {
        if (word_to_document_frequency_.count(minus_word) == 0)
        {
            continue;
        }
        if (word_to_document_frequency_.at(minus_word).count(document_id))
        {
            return std::tuple(plus_words_in_document, document_data_.at(document_id).status);
        }
    }

    // если минус-слов не нашли, то переходим к плюс-словам
    for (const std::string &plus_word : query.plus_words)
    {
        if (word_to_document_frequency_.count(plus_word) == 0)
        {
            continue;
        }
        if (word_to_document_frequency_.at(plus_word).count(document_id))
        {
            plus_words_in_document.push_back(plus_word);
        }
    }
    return std::tuple(plus_words_in_document, document_data_.at(document_id).status);
}

int SearchServer::GetDocumentCount() const
{
    return document_data_.size();
}

std::set<int>::const_iterator SearchServer::begin() const
{
    return added_documents_.begin();
}

std::set<int>::const_iterator SearchServer::end() const
{
    return added_documents_.end();
}

bool SearchServer::IsStopWord(const std::string &word) const
{
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string &text) const
{
    std::vector<std::string> words;
    for (const std::string &word : SplitIntoWords(text))
    {
        if (!IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

// функция обработки слова (отбрасываем минус если он есть), и постановки флагов минус-слова и стоп-слова
SearchServer::QueryWord SearchServer::ProcessQueryWord(std::string raw_word) const 
{
    bool is_minus_word = 0;
    if (raw_word.front() == '-')
    {
        raw_word.erase(0, 1);
        is_minus_word = 1;
        if (raw_word.empty())
        {
            throw std::invalid_argument("There must be a word after minus sign in the query"s);
        }
    }
    if (!IsValidWord(raw_word))
    {
        throw std::invalid_argument("Query word must not contain special characters"s);
    }

    if (raw_word.front() == '-' || raw_word.back() == '-')
    {
        throw std::invalid_argument("There must not be two minus signs before a word, and no minus signs after the word"s);
    }

    bool is_stop_word = IsStopWord(raw_word);
    return {raw_word, is_minus_word, is_stop_word};
}

// возвращаем множества плюс- и минус- слов
SearchServer::ProcessedQuery SearchServer::ParseQuery(const std::string &text) const 
{
    std::set<std::string> plus_words;
    std::set<std::string> minus_words;
    for (const std::string &word : SplitIntoWords(text))
    {
        const QueryWord word_struct = ProcessQueryWord(word);

        if (word_struct.is_stop_word)
        {
            continue;
        }
        if (word_struct.is_minus_word)
        {
            minus_words.insert(word_struct.word);
        }
        else
        {
            plus_words.insert(word_struct.word);
        }
    }
    return {plus_words, minus_words};
}

double SearchServer::CalculateIDF(const std::string &word) const // считаем IDF слова
{
    if (word_to_document_frequency_.count(word))
    {
        int times_word_in_documents = word_to_document_frequency_.at(word).size();
        return std::log(static_cast<double>(document_data_.size()) / times_word_in_documents);
    }
    else
    {
        return 0;
    }
}

int SearchServer::ComputeAverageRating(const std::vector<int> &ratings)
{
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

bool SearchServer::IsValidWord(const std::string &text)
{
    return std::none_of(text.begin(), text.end(), [](char c)
                        { return c >= '\0' && c < ' '; });
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    if (doc_id_to_word_frequency_.count(document_id))
    {
        return doc_id_to_word_frequency_.at(document_id);
    }
    static std::map<std::string, double> empty_map_;
    return empty_map_;
}

void SearchServer::RemoveDocument(int document_id)
{
    for (const auto& [word, freq] : doc_id_to_word_frequency_.at(document_id))
    {
        word_to_document_frequency_.at(word).erase(document_id);
        if (word_to_document_frequency_.at(word).empty())
        {
            word_to_document_frequency_.erase(word);    
        }
    }    
    document_data_.erase(document_id);
    added_documents_.erase(document_id);
    doc_id_to_word_frequency_.erase(document_id);
}