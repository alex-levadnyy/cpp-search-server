// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

// Тест проверяет, что поисковая система правильно расчитывает релеватность
void TestRelevance() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    vector rat = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
    const double result_relevance = 0.23104906018664842;
    double test_relevance = rat[0].relevance;
    ASSERT(test_relevance == result_relevance);

}

// Тест проверяет, что поисковая система правильно расчитывает рейтинг
void TestRating() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    vector rat = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
    const int result_relevance = 9;
    int test_relevance = rat[0].rating;
    ASSERT_EQUAL(test_relevance, result_relevance);

}
// Тест проверяет, что поисковая система правильно возвращает найденные слова в документе из поискового запроса
void TestMatchDocument() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    const vector rat = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
    const auto& test_relevance = get<0>(search_server.MatchDocument("пушистый ухоженный кот"s, 0));
    const vector<string> result_relevance = { "кот"s };
    ASSERT_EQUAL_HINT(test_relevance, result_relevance, "Testing the MatchDocument function"s);

}


// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    TestRelevance();
    TestRating();
    TestMatchDocument();
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------
