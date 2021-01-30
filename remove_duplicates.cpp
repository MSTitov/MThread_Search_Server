#include "remove_duplicates.h"

bool isDuplicate(const std::vector<std::string>& document_1_words, const std::vector<std::string>& document_2_words) {
    if (document_1_words.size() != document_2_words.size()) return false;

    for (int i = 0; i < document_1_words.size(); i++) {
        if (document_1_words[i] != document_2_words[i]) {
            return false;
        }
    }

    return true;
}

void RemoveDuplicates(const SearchServer& search_server) {
    std::vector<std::pair<int, std::vector<std::string>>> ids_to_words;
    for (int document_id : search_server) {
        std::vector<std::string> words;
        auto words_to_freqs = search_server.GetWordFrequencies(document_id);
        for (auto element : words_to_freqs) {
            words.push_back(element.first);
        }
        ids_to_words.push_back({ document_id, words });
    }

    std::set<int> duplicate_ids;

    for (int i = 0; i < ids_to_words.size(); ++i) {
        for (int j = i + 1; j < ids_to_words.size(); ++j) {
            if (isDuplicate(ids_to_words[i].second, ids_to_words[j].second)) {
                duplicate_ids.insert(j + 1);
            }
        }
    }

    for (int id : duplicate_ids) {
        std::cout << "Found duplicate document id " << id << std::endl;
        search_server.RemoveDocument(id);
    }
}