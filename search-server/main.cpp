/*Программа представляет собой прототип поисковой системы.
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
*			document.h					- печать документовmain.cpp
			paginator.h					- вывод результатов по страницамread_input_functions.cpp
			read_input_functions.h		- чтение данных из командной строкиrequest_queue.cpp
			request_queue.h				- обработка запросов на которые ничего не нашлосьsearch_server.cpp
			search_server.h				- основной функционал поисковой системыstring_processing.cpp
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


#include "document.h"
#include "paginator.h"
#include "read_input_functions.h"
#include "request_queue.h"
#include "string_processing.h"
#include "log_duration.h"
#include "remove_duplicates.h"
#include "test_example_functions.h"
#include "search_server.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    SearchServer search_server("and with"s);

    AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // дубликат документа 2, будет удалён
    AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // отличие только в стоп-словах, считаем дубликатом
    AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // множество слов такое же, считаем дубликатом документа 1
    AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // добавились новые слова, дубликатом не является
    AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
    AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

    // есть не все слова, не является дубликатом
    AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // слова из разных документов, не является дубликатом
    AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
    RemoveDuplicates(search_server);
    cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;

    return 0;
}
