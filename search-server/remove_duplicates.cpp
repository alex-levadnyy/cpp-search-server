#include "remove_duplicates.h"

#include <set>
#include <vector>


void RemoveDuplicates(SearchServer& search_server) {
    std::set<std::set<std::string_view, std::less<>>> unique_docs;
    std::vector<int> docs_to_delete;
    for (const int id : search_server) {
        const std::map<std::string_view, double> word_freqs = search_server.GetWordFrequencies(id);
        std::set<std::string_view, std::less<>> words;
        transform(word_freqs.begin(), word_freqs.end(), inserter(words, words.begin()),
            [](const std::pair<std::string_view, double> word) {
                return word.first;
            });

        if (unique_docs.count(words) == 0) {
            unique_docs.insert(words);
        }
        else {
            docs_to_delete.push_back(id);
        }
    }

    for (const int id : docs_to_delete) {
        std::cout << "Found duplicate document id " + std::to_string(id) << "\n";
        search_server.RemoveDocument(id);
    }
}