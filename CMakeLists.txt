cmake_minimum_required(VERSION 3.11)
project(SearchServer CXX)
set(CMAKE_CXX_STANDARD 17)

set (search_server
    "search-server/concurrent_map.h"

    "search-server/document.h"
    "search-server/document.cpp"

    "search-server/paginator.h"

    "search-server/process_queries.h"
    "search-server/process_queries.cpp"

    "search-server/read_input_functions.h"
    "search-server/read_input_functions.cpp"

    "search-server/request_queue.h"
    "search-server/request_queue.cpp"

    "search-server/remove_duplicates.h"
    "search-server/remove_duplicates.cpp"

    "search-server/search_server.h"
    "search-server/search_server.cpp"

    "search-server/string_processing.h"
    "search-server/string_processing.cpp"
    )

add_executable(SearchServer search-server/main.cpp ${search_server})

target_include_directories(SearchServer PRIVATE "search-server")

if (MSVC)
    target_compile_options(SearchServer PRIVATE /W3)
else ()
    target_compile_options(SearchServer PRIVATE
        -ltbb
        -lpthread
        -Werror
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
        -pedantic
        -pedantic-errors
        )
endif ()