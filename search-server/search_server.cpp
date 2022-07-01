#include "search_server.h"

using namespace std;

/*SearchServer::SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWordsView(stop_words_text))  // Invoke delegating constructor from string container
{
    for (const auto& word : stop_words_) {
        if (!IsValidWord(word)) {
            //            throw invalid_argument("invalid characters in stopword \""s + string(word) + "\""s);
            throw invalid_argument("invalid characters in stopword "s);
        }
    }
}

SearchServer::SearchServer(const string_view stop_words_text)
    : SearchServer(SplitIntoWordsView(stop_words_text))  // Invoke delegating constructor from string container
{
    for (const auto& word : stop_words_) {
        if (!IsValidWord(word)) {
            //            throw invalid_argument("invalid characters in stopword \""s + string(word) + "\""s);
            throw invalid_argument("invalid characters in stopword "s);
        }
    }
}*/



void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }
    const auto [it, inserted] = documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status, std::string(document) });
	const auto words = SplitIntoWordsNoStop(it->second.text);
    const double inv_word_count = 1.0 / words.size();
    for (string_view word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_to_word_freqs_[document_id][word] += inv_word_count;
    }    
    document_ids_.insert(document_id);
}

/*void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status,
                               const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status, string(document) });
    const auto words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    auto& word_freqs = document_to_word_freqs_[document_id];
    for (string_view word : words) {
        auto it = all_words.insert(string(word)); // make a word hard copy
        string_view sw = *it.first;
        word_to_document_freqs_[sw][document_id] += inv_word_count;
        word_freqs[sw] += inv_word_count;        
    }
    document_ids_.insert(document_id);
}*/



vector<Document> SearchServer::FindTopDocuments(const string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(const string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}


int SearchServer::GetDocumentCount() const {
    return static_cast<int>(document_ids_.size());
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static map<string_view, double> result;
    if (document_to_word_freqs_.count(document_id) != 0) {
        for (auto [word, freq] : document_to_word_freqs_.at(document_id)) {
            result.emplace(word, freq);
        }
    }
    return result;
}


void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
    if (documents_.count(document_id)) {
        for (auto& word : document_to_word_freqs_[document_id]) {
            word_to_document_freqs_[word.first].erase(document_id);
        }
        documents_.erase(document_id);
        document_ids_.erase(document_id);
        document_to_word_freqs_.erase(document_id);
    }
}

void SearchServer::RemoveDocument(const execution::parallel_policy&, int document_id) {
    if (document_ids_.count(document_id) != 0) {
        documents_.erase(document_id);
        document_ids_.erase(document_id);
        for_each(
            execution::par,
            word_to_document_freqs_.begin(), word_to_document_freqs_.end(),
            [document_id](auto& document) {document.second.erase(document_id); });
        document_to_word_freqs_.erase(document_id);
    }
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq, document_id);
}

/*tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, const string_view raw_query, int document_id) const {

    Query query = ParseQuery(raw_query);
    vector<string_view> matched_words;
    for (const string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status };
}*/

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const {
    if (!document_ids_.count(document_id)) {
        throw std::out_of_range("incorrect document_id");
    }
    const Query query = ParseQuery(raw_query);
 
    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return { {}, documents_.at(document_id).status };
        }
    }
    std::vector<std::string_view> matched_words;
 
    for (const std::string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const string_view raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const execution::parallel_policy&, const string_view raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query, true);
    //const auto query = ParseQuery(raw_query);
    const auto status = documents_.at(document_id).status;
    const auto word_checker =
        [this, document_id](const string_view word) {
        const auto it = word_to_document_freqs_.find(word);
        return it != word_to_document_freqs_.end() && it->second.count(document_id);
    };
    vector<string_view> empty;
    if (any_of(execution::par, query.minus_words.begin(), query.minus_words.end(), word_checker)) {
        return { empty, status };
    }

    vector<string_view> matched_words(query.plus_words.size());
    auto words_end = copy_if(
        execution::par,
        query.plus_words.begin(), query.plus_words.end(),
        matched_words.begin(),
        word_checker
    );
    sort(matched_words.begin(), words_end);
    words_end = unique(matched_words.begin(), words_end);
    matched_words.erase(words_end, matched_words.end());
    return { matched_words, status };
}

/*
const auto query = ParseQuery(raw_query);

        const auto checker = [this, document_id](std::string_view word) {
            return word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id);
        };

        std::vector<std::string_view> matched_words;
        matched_words.reserve(query.plus_words.size());

        for_each(policy, query.plus_words.begin(), query.plus_words.end(),
                 [&checker, &matched_words](std::string_view word) {
                     if (checker(word)) {
                         matched_words.push_back(std::move(word));
                     }
                 });

        const auto status = documents_.at(document_id).status;

        if (any_of(policy, query.minus_words.begin(), query.minus_words.end(), [&checker](std::string_view word) { return checker(word); })) {
            return { {}, status };
        }

        return { matched_words, status };
*/


bool SearchServer::IsStopWord(string_view word) const {
    return stop_words_.count(string(word)) > 0;
}

bool SearchServer::IsValidWord(const string_view word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(const string_view text) const {
    vector<string_view> words;
    for (const string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            //            throw std::invalid_argument("Word "s + string(word) + " is invalid"s);
            throw std::invalid_argument("Word is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    return accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}


SearchServer::Query SearchServer::ParseQuery(const string_view text, bool skip_sort) const {
    Query result;
    for (const string_view word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            }
            else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }
    if (!skip_sort) {
        for (auto* words : { &result.plus_words, &result.minus_words }) {
            sort(words->begin(), words->end());
            words->erase(unique(words->begin(), words->end()), words->end());
        }
    }
    return result;
}

SearchServer::Query SearchServer::ParseQuery(const string_view text) const {
    return ParseQuery(text, false);
}

/*SearchServer::Query SearchServer::ParseQuery(string_view text) const {
    SearchServer::Query result;
    for (string_view word : SplitIntoWordsView(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            } else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }
    return result;
}*/

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    //std::string_view word = text;
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        //text = text.substr(1);
        text.remove_prefix(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        //        throw std::invalid_argument("Query word "s + string(text) + " is invalid"s);
        throw std::invalid_argument("Query word is invalid"s);
    }

    return { text, is_minus, IsStopWord(text) };
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string_view word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}