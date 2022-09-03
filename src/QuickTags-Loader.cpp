#include "QuickTags-Loader.hpp"
#include <sstream>

//#include "stdio.h"

void BuildTagStringSetFromFile(std::fstream& inFile, std::set<std::string>& outStringSet)
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

void SplitString(const std::string& inStr, std::vector<std::string>& outSubStrs)
{
  std::stringstream ss(inStr);

  for (std::string subStr; std::getline(ss, subStr, '.'); )
  {
    outSubStrs.push_back(subStr);
  }
}

void TreeifyTags(const std::set<std::string>& inStringSet, std::vector<TagTreeNode>& outTagTrees)
{
  for (const std::string& tagString : inStringSet)
  {
    printf("Tag String: %s\n", tagString.c_str());
    // Split tag string into components
    std::vector<std::string> subStrings;
    SplitString(tagString, subStrings);

    // Shouldn't happen, but guard against it anyway
    if (subStrings.size() == 0)
    {
      continue;
    }
    
    std::vector<std::string>::const_iterator subStrIt = subStrings.begin();

    // Check if top-level node exists
    const std::string& first = *subStrIt;
    std::vector<TagTreeNode>::iterator tagTreeIt = std::find(outTagTrees.begin(), outTagTrees.end(), first);
    if (tagTreeIt == outTagTrees.end())
    {
      printf("%s top-level tag not seen before, creating\n", first.c_str());

      // Create new top-level node
      outTagTrees.emplace_back(subStrings[0]);
      if (subStrings.size() == 1)
      {
        continue;
      }
      tagTreeIt = --outTagTrees.end();
    }

    //// Create new top-level node
    //if (subStrings.size() == 1)
    //{
    //  outTagTrees.emplace_back(subStrings[0]);
    //  continue;
    //}

    //// Find top-level tag
    //const std::string& first = subStrings[0];
    //std::vector<TagTreeNode>::iterator tagTreeIt = std::find(outTagTrees.begin(), outTagTrees.end(), first);
    //if (tagTreeIt == outTagTrees.end())
    //{
    //  printf("Tag %s out of order somehow?\n", tagString.c_str());
    //  continue;
    //}

    // Else matched top-level tag, add sub-tags to top-level tag, following the chain down    

    //subStrIt++; // Step forward to first sub-tag string

    // Working sub-tag
    std::vector<TagTreeNode>::iterator subTagTreeIt;

    // Starting values
    TagTreeNode* node = &*tagTreeIt;
    std::vector<TagTreeNode>* subTags = &node->SubTags;

    // For each substring, walk down the tree, adding new nodes as needed
    for (++subStrIt; subStrIt != subStrings.end(); ++subStrIt)
    {
      // Does this sub-tag exist on this node?
      printf("Current Node: %s\n", node->Tag.c_str());
      printf("\tLooking for %s\n", subStrIt->c_str());
      subTagTreeIt = std::find(subTags->begin(), subTags->end(), *subStrIt);
      if (subTagTreeIt == subTags->end())
      {
        printf("\t%s not seen before, emplacing below %s\n", subStrIt->c_str(), node->Tag.c_str());

        // Tag not seen before, add to list
        node = &(subTags->emplace_back(*subStrIt));
        
        // Fast-path, add rest of sub strings going down from this node
        for (++subStrIt; subStrIt != subStrings.end(); ++subStrIt)
        {
          printf("\tEmplacing %s below %s\n", subStrIt->c_str(), node->Tag.c_str());

          node = &(node->SubTags.emplace_back(*subStrIt));
        }
        break;
      }
      else
      {
        printf("\t%s seen before, switching to that node\n", subTagTreeIt->Tag.c_str());
        // Update working data
        node = &*subTagTreeIt;
        subTags = &node->SubTags;

        // continue to next substring
        continue;
      }
    }
  }
}
