#pragma once
//#include "QuickTags.hpp"

#include <string>
#include <fstream>
#include <set>
#include <vector>
#include <span>

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

void EnumerateTags(std::vector<TagTreeNode>& tags);

void FindTagRanges(const std::vector<TagTreeNode>& inTags, std::vector<unsigned int>& outRanges);

void GetRequiredBitsPerField(const std::vector<unsigned int>& fieldRanges, std::vector<unsigned int>& outBits);

enum class EQTagIntBase
{
  UInt8,
  UInt16,
  UInt32,
  UInt64
};
EQTagIntBase FindSmallestIntBase(const std::vector<unsigned int>& inBits);

std::string GetTemplateString(const EQTagIntBase base, const std::vector<unsigned int>& fieldBits);
