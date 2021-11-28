// -------- Начало модульных тестов поисковой системы ----------

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

/*
Разместите код остальных тестов здесь
*/

//Тестирование расчета релевантности и сортировки документов в выдаче по релевантности
void TestRelevance() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    vector relevance = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL);
    const vector <double> right_relevance = { 0.86643397569993164, 0.17328679513998632, 0.17328679513998632 };
    vector <double> test_relevance;
    for (const Document& doc : relevance) {
        test_relevance.push_back(doc.relevance);
    };
    ASSERT_EQUAL(test_relevance, right_relevance);
}

//Тестирование расчета рейтинга документов
void TestRating() {
    const string content = "cat in the city"s;
    {
        SearchServer search_server;
        search_server.SetStopWords("in into"s);
        search_server.AddDocument(0, content, DocumentStatus::ACTUAL, { 9 });
        int right_rating = 9;
        int test_rating = search_server.FindTopDocuments("fluffy cat"s, DocumentStatus::ACTUAL)[0].rating;
        ASSERT_EQUAL(test_rating, right_rating);
    }

    {
        SearchServer search_server;
        search_server.SetStopWords("in into"s);
        search_server.AddDocument(0, content, DocumentStatus::ACTUAL, { 8, -3 });
        int right_rating = 2;
        int test_rating = search_server.FindTopDocuments("fluffy cat"s, DocumentStatus::ACTUAL)[0].rating;
        ASSERT_EQUAL_HINT(test_rating, right_rating, "Testing a rating with negative data"s);
    }

    {
        SearchServer search_server;
        search_server.SetStopWords("in into"s);
        search_server.AddDocument(0, content, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(1, content, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
        const vector <int> right_rating = { 5, -1 };
        vector rel = search_server.FindTopDocuments("fluffy cat"s, DocumentStatus::ACTUAL);
        vector <int> test_rating;
        for (const Document& doc : rel) {
            test_rating.push_back(doc.rating);
        };
        ASSERT_EQUAL_HINT(test_rating, right_rating, "Rating vector testing"s);
    }
}

//Тестирование добавления документов
void TestAddDocument() {
    const string content = "белый кот и модный ошейник"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = search_server.FindTopDocuments("кот без прикрас"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
    
        search_server.AddDocument(1, content, DocumentStatus::ACTUAL, ratings);
        search_server.AddDocument(2, content, DocumentStatus::ACTUAL, ratings);
        search_server.AddDocument(3, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs_ = search_server.FindTopDocuments("кот без забот"s);
        ASSERT_EQUAL_HINT(found_docs_.size(), 4u, "Testing adding documents"s);
    }
}

//Тестирование совпадения слов из запроса и документа
void TestMatchDocument() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    auto test_match_document = get<0>(search_server.MatchDocument("пушистый ухоженный кот"s, 0));
    vector<string> result_match_document = { "кот"s, };
    ASSERT_EQUAL(test_match_document, result_match_document);
}

//Тестирование фильтрации по статусу документа
void TestFilterStatusDocument() {
    SearchServer search_server;
    search_server.SetStopWords("и в на за из"s);
    const vector<int> ratings = { 1, 2, 3 };

    search_server.AddDocument(0, "кот весь день у батареи"s, DocumentStatus::ACTUAL, ratings);
    search_server.AddDocument(1, "то бока, то лапы греет"s, DocumentStatus::ACTUAL, ratings);
    search_server.AddDocument(2, "он из кухни в холода"s, DocumentStatus::BANNED, ratings);
    search_server.AddDocument(3, "не выходит никуда"s, DocumentStatus::BANNED, ratings);
    search_server.AddDocument(4, "нет, напрасно мы решили"s, DocumentStatus::IRRELEVANT, ratings);
    search_server.AddDocument(5, "прокатить кота в машине"s, DocumentStatus::BANNED, ratings);
    search_server.AddDocument(6, "кот кататься не привык"s, DocumentStatus::REMOVED, ratings);
    search_server.AddDocument(7, "опрокинул грузовик"s, DocumentStatus::REMOVED, ratings);
    search_server.AddDocument(8, "спасибо за ревью!"s, DocumentStatus::ACTUAL, { 10, 10 });

    {
        vector test_status_doc = search_server.FindTopDocuments("спасибо"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL_HINT(test_status_doc.size(), 1u, "Testing a filter by document status"s);
    }

    {
        vector test_status_doc = search_server.FindTopDocuments("кот выходит из кухни"s, DocumentStatus::BANNED);
        ASSERT_EQUAL_HINT(test_status_doc.size(), 2u, "Testing a filter by document status"s);
    }

    {
        vector test_status_doc = search_server.FindTopDocuments("кот угнал грузовик"s, DocumentStatus::REMOVED);
        ASSERT_EQUAL_HINT(test_status_doc.size(), 2u, "Testing a filter by document status"s);
    }
}
    
// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    TestRelevance();
    TestRating();
    TestMatchDocument();
    TestAddDocument();
    TestFilterStatusDocument();
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------
