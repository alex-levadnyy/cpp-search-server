#include "remove_duplicates.h"
#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace std::literals;

void RemoveDuplicates(SearchServer& search_server) {
	std::vector<int> id_document_for_delete;
	std::set<std::vector<std::string>> word_doc;
	for (int id_document : search_server) {
		const auto& id_word_freq = search_server.GetWordFrequencies(id_document);
		std::vector<std::string> temp;
		for (const auto& word : id_word_freq) {
			temp.push_back(word.first);
		}
		if (word_doc.count(temp)) {
			id_document_for_delete.push_back(id_document);
		}
		else {
			word_doc.insert(temp);
		}
	}
	for (int i = 0; i != id_document_for_delete.size(); ++i) {
		search_server.RemoveDocument(id_document_for_delete[i]);
		std::cout << "Found duplicate document id "s << id_document_for_delete[i] << std::endl;
	}
}