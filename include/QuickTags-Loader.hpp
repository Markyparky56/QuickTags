#pragma once
#include "QuickTags.hpp"

#include <string>
#include <fstream>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <span>
#include <sstream>

namespace QTagUtil
{
  void BuildTagStringSetFromFile(std::fstream& inFile, std::set<std::string>& outStringSet);

  struct TagTreeNode
  {
    std::string Tag;
    std::uint64_t TagAsInt;
    std::list<TagTreeNode> SubTags;
    TagTreeNode* ParentTag;

    TagTreeNode(const std::string& tag)
      : Tag(tag)
      , TagAsInt(0)
      , ParentTag(nullptr)
    {}

    bool operator==(const TagTreeNode& other) const
    {
      return other.Tag == Tag;
    }
  };

  void TreeifyTags(const std::set<std::string>& inStringSet, std::list<TagTreeNode>& outTagTrees);

  void EnumerateTags(std::list<TagTreeNode>& tags);

  void FindTagRanges(const std::list<TagTreeNode>& inTags, std::vector<unsigned int>& outRanges);

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
  
  namespace Internal
  {
    void GetAllSubTags(const TagTreeNode& topLevelNode, std::vector<const TagTreeNode*>& outNodes);

    template<class QTag>
    void GetEachTagAsQTag(const TagTreeNode& topLevelNode, std::map<QTag, std::string>& outTagStringMap, std::vector<QTag> outTags)
    {
      using TagStringPair = std::pair<QTag, std::string>;
      using StringValuesPair = std::pair<std::string, std::vector<std::uint64_t>>;

      std::vector<const TagTreeNode*> allNodes;
      allNodes.push_back(&topLevelNode);
      GetAllSubTags(topLevelNode, allNodes);

      // For each node in all nodes
      for (const TagTreeNode* node : allNodes)
      {
        // Collect parents, walking backwards until finding top-level root
        std::vector<const TagTreeNode*> parents;
        const TagTreeNode* parent = node->ParentTag;
        while (parent)
        {
          parents.push_back(parent);
          parent = parent->ParentTag;
        }

        // Re-assemble tag string
        std::stringstream ss;
        std::vector<typename QTag::TagBaseType> tagFieldValues;
        for (std::vector<const TagTreeNode*>::reverse_iterator rit = parents.rbegin(); rit != parents.rend(); ++rit)
        {
          const TagTreeNode& node = **rit;
          ss << node.Tag.c_str() << '.';
          tagFieldValues.push_back((typename QTag::TagBaseType)node.TagAsInt);
        }
        ss << node->Tag.c_str();
        tagFieldValues.push_back((typename QTag::TagBaseType)node->TagAsInt);
        std::string tagString = ss.str();
        
        QTag tag;
        for (int i = (int)tagFieldValues.size() - 1; i >= 0; --i)
        {
          tag.SetField((unsigned char)i, tagFieldValues[i]);
        }
        char* tagAsString = tag.ValueAsString();

        printf("%s: \t\t%s\n", tagString.c_str(), tagAsString);
        delete[] tagAsString;

        // TODO: Create QTag
        // Add to outTags
        // Add to outTagStringMap
      }
    }
  }  

  template<class QTag>
  void LoadQuickTagsFromFile(std::fstream& inFile, std::map<QTag, std::string>& outTagStringMap, std::vector<QTag>& outTags)
  {
    // Find all unique, valid tag strings
    std::set<std::string> stringSet;
    BuildTagStringSetFromFile(inFile, stringSet);

    // Build tree from split tags, enumerate (assign value to each sub-tag)
    std::list<TagTreeNode> tagTree;
    TreeifyTags(stringSet, tagTree);
    EnumerateTags(tagTree);

    // For each node in tree, create corresponding tag
    for (const TagTreeNode& topLevelNode : tagTree)
    {
      Internal::GetEachTagAsQTag(topLevelNode, outTagStringMap, outTags);
    }
  }
}
