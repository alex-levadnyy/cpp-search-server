#include "process_queries.h"

#include <algorithm>
#include <execution>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    
    std::vector<std::vector<Document>> result (queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(),[&search_server](std::string query){
        return search_server.FindTopDocuments(query);
    });
    
    return result;
} 

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    std::vector<std::vector<Document>> processqueries = ProcessQueries(search_server, queries);
    size_t size_result = 0;
    for (const auto& item : processqueries) {
        size_result += item.size();
    }
    
    std::vector<Document> result;
    result.reserve(size_result);
    for (const auto& document : processqueries){
        for (const auto& data : document){
            result.push_back(data);
        }
    }
    return result;
}