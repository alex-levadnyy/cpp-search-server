/*Программа представляет собой прототип поисковой системы.
* ver. 0.4.0
* - добавлены паралленльные версии методов FindTopDocuments, FindAllDocuments, RemoveDocument
* - добавлено использование string_view
*
* ver. 0.3.1
* - добавлен метод class SearchServer - GetWordFrequencies : определяет частоту слова в документе
* - добавлен метод class SearchServer - RemoveDocument : удаляет дубликаты документов
* - добавлена свободная функция - RemoveDuplicates : ищет и удаляет дубликаты документов
* - добавлен класс LogDuration - определение времени работы функций и методов
* - новые файлы проекта:
            log_duration.h              - определение времени работы
            remove_duplicates.h         - функция поиска и удаления дубликатов документов
            test_example_functions.h    - тестирование функционала
*
* ver. 0.3.0
* - написан class RequestQueue: обработка запросов на которые ничего не нашлось
* - написан class Paginator: для постраничного вывода результата поиска
* - проект разбит на файлы:
*	    document.h				- печать документовmain.cpp
            paginator.h				- вывод результатов по страницамread_input_functions.cpp
            read_input_functions.h		- чтение данных из командной строкиrequest_queue.cpp
            request_queue.h			- обработка запросов на которые ничего не нашлосьsearch_server.cpp
            search_server.h			- основной функционал поисковой системыstring_processing.cpp
            string_processing.h			- работа со строками
*
* ver. 0.2.1
* - функции SplitIntoWordsNoStop и ParseQuery возвращают результат
* - выброс исключений происходит в обрабатывающих функциях
* - более подробная информация о причинах срабатывания исключений
*
* ver. 0.2.0
* - Добавлена обработка ошибок:
*       + исключается добавление документов со специсимволами
*       + исключается добавление документов с одинковым или отрицательным ID
*       + исключается обработка поисковых запросов со спецсимволами,
*            '--' перед словом, '-' без слова после
*
*
*
* ver. 0.1.0
* Реализован class SearchServer.
* FindTopDocuments - метод для поиска по покуметам с необходимыми критериями.
*   На входи может принимать:
*       - минус-слова, поисковый запрос (по умолчанию выводит документы со статусом ACTUAL)
*       - минус-слова, поисковый запрос, статус документа
*       - минус-слова, поисковый запрос, статус документа, предикат
*       - минус-слова, поисковый запрос, предикат
* Вычисляет рейтинг документа, TF-IDF
* Выводит количество документов определенное const  MAX_RESULT_DOCUMENT_COUNT
*
* Старт обучения 01.11.2021 ЯПрактикум
*/


#include "search_server.h"
#include "process_queries.h"

#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    std::uniform_int_distribution<int> distribution('a', 'z');
    std::string word(length, ' ');
    for (char& c : word) {
        c = char(distribution(generator));
    }
    return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}

template <typename ExecutionPolicy>
void Test(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    double total_relevance = 0;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
        }
    }
    cout << total_relevance << endl;
}

#define TEST(policy) Test(#policy, search_server, queries, execution::policy)

int main() {
    mt19937 generator;

    const auto dictionary = GenerateDictionary(generator, 1000, 10);
    const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
    }

    const auto queries = GenerateQueries(generator, dictionary, 100, 70);

    TEST(seq);
    TEST(par);
}
