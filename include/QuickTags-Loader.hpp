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
  enum class ETagSetFlags : unsigned int
  {
    None = 0,
    CaseInsensitive = (1 << 0),
  };

  void BuildTagStringSetFromFiles(std::vector<std::fstream>& inFiles, std::set<std::string>& outStringSet, ETagSetFlags flags=ETagSetFlags::None);
  void BuildTagStringSetFromFile(std::fstream& inFile, std::set<std::string>& outStringSet, ETagSetFlags flags=ETagSetFlags::None);

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

  void FindTagRanges(const std::list<TagTreeNode>& inTags, std::vector<std::uint32_t>& outRanges);

  void GetRequiredBitsPerField(const std::vector<std::uint32_t>& fieldRanges, std::vector<std::uint32_t>& outBits);

  enum class EQTagIntBase
  {
    UInt8,
    UInt16,
    UInt32,
    UInt64
  };
  EQTagIntBase FindSmallestIntBase(const std::vector<std::uint32_t>& inBits);

  std::string GetTemplateString(const EQTagIntBase base, const std::vector<std::uint32_t>& fieldBits);
  
  namespace Internal
  {
    void GetAllSubTags(const TagTreeNode& topLevelNode, std::vector<const TagTreeNode*>& outNodes);

    template<class QTag>
    void GetEachTagAsQTag(const TagTreeNode& topLevelNode, std::map<QTag, std::string>& outTagStringMap, std::vector<QTag>& outTags)
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
          // Collect value as well
          tagFieldValues.push_back((typename QTag::TagBaseType)node.TagAsInt);
        }
        ss << node->Tag.c_str();
        tagFieldValues.push_back((typename QTag::TagBaseType)node->TagAsInt);
        std::string tagString = ss.str();
        
        // Loop backwards over values array to set each field in tag
        QTag tag;
        for (int i = (int)tagFieldValues.size() - 1; i >= 0; --i)
        {
          tag.SetField((unsigned char)i, tagFieldValues[i]);
        }

#ifdef QTAG_DEBUGSTRINGS
        char* tagAsString = tag.ValueAsString();
        printf("%s:\t\t%s\n", tagString.c_str(), tagAsString);
        delete[] tagAsString;
#endif

        // Add to outTags
        outTags.push_back(tag);
        
        // Add to outTagStringMap
        outTagStringMap.emplace(tag, tagString);
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
