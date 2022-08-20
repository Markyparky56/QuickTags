#include "QuickTags-Loader.hpp"

#include "stdio.h"

void BuildTagStringSetFromFile(std::fstream& inFile, std::unordered_set<std::string>& outStringSet)
{
  for (std::string line; std::getline(inFile, line); )
  {
    // Validate string
    const bool noSpaces = line.find_first_of(' ') == -1;
    const bool noTabs = line.find_first_of('\t') == -1;

    if (noSpaces && noTabs)
    {
      outStringSet.insert(line);
    }
  }
}
