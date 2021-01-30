#pragma once

#include "search_server.h"
#include <vector>
#include <string>

bool isDuplicate(std::vector<std::string> document_1_words, std::vector<std::string> document_2_words);

void RemoveDuplicates(SearchServer& search_server);