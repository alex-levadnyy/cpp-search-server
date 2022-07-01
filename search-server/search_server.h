#pragma once

#include "document.h"
#include "string_processing.h"
#include "concurrent_map.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <functional>
#include <numeric>
#include <execution>
#include <future>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
inline static constexpr double EPSILON = 1e-6;

class SearchServer {
public:

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    using namespace std::string_literals;
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

   // explicit SearchServer(const std::string& stop_words_text);
   // explicit SearchServer(const std::string_view stop_words_text);
    
    explicit SearchServer(std::string stop_words_text)
        : SearchServer(std::string_view(stop_words_text)) {
    }

    explicit SearchServer(std::string_view stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)) {
    }
    
    
    void AddDocument(int document_id, std::string_view document,
        DocumentStatus status = DocumentStatus::ACTUAL, const std::vector<int>& ratings = {});

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const;
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(const std::string_view raw_query) const;

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(ExecutionPolicy policy, const std::string_view raw_query, DocumentPredicate document_predicate) const;
    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy policy, const std::string_view raw_query, DocumentStatus status) const;
    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy policy, const std::string_view raw_query) const;
    
    int GetDocumentCount() const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    
    void RemoveDocument(int document_id);
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);
    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);

    auto begin() const {
        return document_ids_.cbegin();
    }

    auto end() const {
        return document_ids_.cend();
    }

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, const std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, const std::string_view raw_query, int document_id) const;
      



private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
        std::string text;
    };
    //const std::set<std::string> stop_words_;
    const std::set<std::string, std::less<>> stop_words_;
   // std::unordered_set<std::string> all_words;

    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_to_word_freqs_;

    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;

    bool IsStopWord(std::string_view word) const;

    static bool IsValidWord(const std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    //QueryWord ParseQueryWord(const std::string& text) const;
    QueryWord ParseQueryWord(std::string_view text) const;

    //    struct Query {
    //        std::set<std::string> plus_words;
    //        std::set<std::string> minus_words;
    //    };
    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };



    Query ParseQuery(const std::string_view text) const;
    Query ParseQuery(const std::string_view text, bool skip_sort) const;


    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::sequenced_policy, const Query& query, DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::parallel_policy, const Query& query, DocumentPredicate document_predicate) const;

};


template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy policy, const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy policy, const std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy policy, std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    const Query query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(policy, query, document_predicate);

    std::sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            const double EPSILON = 1e-6;
            if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::sequenced_policy, const Query& query, DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (std::string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::parallel_policy, const Query& query, DocumentPredicate document_predicate) const {
    ConcurrentMap<int, double> document_to_relevance_mutex(word_to_document_freqs_.size());
    
    for_each(std::execution::par,
        query.plus_words.begin(), query.plus_words.end(),
        [&document_to_relevance_mutex, &document_predicate, this](const std::string_view word) {
            if (word_to_document_freqs_.count(word) == 0U) {
                return;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance_mutex[document_id].ref_to_value += term_freq * inverse_document_freq;
                }
            }
        });


    std::for_each(std::execution::par,
        query.minus_words.begin(), query.minus_words.end(),
        [&document_to_relevance_mutex, this](const std::string_view word) {
            if (word_to_document_freqs_.count(word) == 0U) {
                return;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance_mutex.erase(document_id);
            }
        });

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance_mutex.BuildOrdinaryMap()) {
        matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}