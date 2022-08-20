#pragma once
//#include "QuickTags.hpp"

#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

void BuildTagStringSetFromFile(std::fstream& inFile, std::unordered_set<std::string>& outStringSet);
