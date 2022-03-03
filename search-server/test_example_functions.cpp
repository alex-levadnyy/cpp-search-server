#include "test_example_functions.h"

void AddDocument(SearchServer& search_server, int document_id, const std::string& document,
    DocumentStatus status, const std::vector<int>& ratings) {
    using namespace std::literals;

    try {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    using namespace std::literals;

    std::cout << "Результаты поиска по запросу: "s << raw_query << std::endl;

    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка поиска: "s << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    using namespace std::literals;

    try {
        std::cout << "Матчинг документов по запросу: "s << query << std::endl;
        for (const int document_id : search_server) {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << std::endl;
    }
}

void TestSearchServer() {
    using namespace std::literals;

    try {
        std::vector <std::string> s1 = { "и"s, "в"s, "на\x12"s };
        SearchServer search_server1(s1);
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка задания стоп-слов : "s << e.what() << std::endl;
    }
}
