#pragma once
//#include "QuickTags.hpp"

#include <string>
#include <fstream>
#include <set>
//#include <unordered_map>
#include <vector>

void BuildTagStringSetFromFile(std::fstream& inFile, std::set<std::string>& outStringSet);

struct TagTreeNode
{
  std::string Tag;
  std::uint64_t TagAsInt;
  std::vector<TagTreeNode> SubTags;

  TagTreeNode(const std::string& tag)
    : Tag(tag)
    , TagAsInt(0)
  {}

  bool operator==(const TagTreeNode& other) const
  {
    return other.Tag == Tag;
  }
};

void TreeifyTags(const std::set<std::string>& inStringSet, std::vector<TagTreeNode>& outTagTrees);
