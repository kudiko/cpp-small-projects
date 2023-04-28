#include "remove_duplicates.h"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

void RemoveDuplicates(SearchServer& search_server)
{
    std::map<std::set<std::string>, int> words_to_id;
    std::vector<int> ids_of_duplicates;

    for (const int doc_id : search_server)
    {
        std::set<std::string> doc_words;
        for (const auto& [word, freq] : search_server.GetWordFrequencies(doc_id))
        {
            doc_words.insert(word);
        }
        if (words_to_id.count(doc_words))
        {
            std::cout << "Found duplicate document id "s << doc_id << std::endl;
            ids_of_duplicates.push_back(doc_id);
        } else {
            words_to_id[doc_words] = doc_id;
        }
    }
    while(!ids_of_duplicates.empty())
    {
        search_server.RemoveDocument(ids_of_duplicates.back());
        ids_of_duplicates.pop_back();
    }
}