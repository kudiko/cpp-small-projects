#include "test_example_functions.h"
#include "log_duration.h"

using namespace std::literals::string_literals;

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {
    std::cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const std::string& word : words) {
        std::cout << ' ' << word;
    }
    std::cout << "}"s << std::endl;
}
 
void AddDocument(SearchServer& search_server, int document_id, const std::string& document,
                 DocumentStatus status, const std::vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const std::exception& e) {
        std::cout << "Error in adding document "s << document_id << ": "s << e.what() << std::endl;
    }
}
 
void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    LOG_DURATION_STREAM("Operation time"s, std::cout);
    std::cout << "Results for request: "s << raw_query << std::endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            std::cout << document << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error is seaching: "s << e.what() << std::endl;
    }
}
 
void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    LOG_DURATION_STREAM("Operation time"s, std::cout);
    try {
        std::cout << "Matching for request: "s << query << std::endl;
        for (const int document_id : search_server)
        {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const std::exception& e) {
        std::cout << "Error in matching request "s << query << ": "s << e.what() << std::endl;
    }
}