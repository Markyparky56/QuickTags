#include "QuickTags-Loader.hpp"
#include <bit>
#include <numeric>
#include <algorithm>
#include <cstdio>

using QTagUtil::TagTreeNode;

#ifdef QTAG_DEBUGSTRINGS
#define QTAG_LOG(...) printf(__VA_ARGS__)
#else
#define QTAG_LOG(...)
#endif

void QTagUtil::BuildTagStringSetFromFiles(std::vector<std::fstream>& inFiles, std::set<std::string>& outStringSet, ETagSetFlags flags)
{
  for (std::fstream& file : inFiles)
  {
    BuildTagStringSetFromFile(file, outStringSet, flags);
  }
}

void QTagUtil::BuildTagStringSetFromFile(std::fstream& inFile, std::set<std::string>& outStringSet, ETagSetFlags flags)
{
  bool bCaseInsensitive = (unsigned int)flags & (unsigned int)ETagSetFlags::CaseInsensitive;

  for (std::string line; std::getline(inFile, line); )
  {
    // Validate string
    const bool noSpaces = line.find_first_of(' ') == -1;
    const bool noTabs = line.find_first_of('\t') == -1;

    if (bCaseInsensitive)
    {
      // toupper line
      // NOTE: For proper text handling, ICU should be used for proper unicode support
      // This transform will only handle basic ascii
      std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c)
        {
          return std::toupper(c);
        });

      // TODO: For aesthetic purposes, we can keep a copy of the original line for doing things like
      // exposing to the user (if they have an editor) and for converting to enums in case we're ruining CamelCase tags
    }

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

void QTagUtil::TreeifyTags(const std::set<std::string>& inStringSet, std::list<TagTreeNode>& outTagTrees)
{
  for (const std::string& tagString : inStringSet)
  {
    QTAG_LOG("Tag String: %s\n", tagString.c_str());
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
    std::list<TagTreeNode>::iterator tagTreeIt = std::find(outTagTrees.begin(), outTagTrees.end(), first);
    if (tagTreeIt == outTagTrees.end())
    {
      QTAG_LOG("%s top-level tag not seen before, creating\n", first.c_str());

      // Create new top-level node
      outTagTrees.emplace_back(subStrings[0]);
      if (subStrings.size() == 1)
      {
        continue;
      }
      tagTreeIt = --outTagTrees.end();
    }

    // Working sub-tag
    std::list<TagTreeNode>::iterator subTagTreeIt;

    // Starting values
    TagTreeNode* node = &*tagTreeIt;
    std::list<TagTreeNode>* subTags = &node->SubTags;

    // For each substring, walk down the tree, adding new nodes as needed
    for (++subStrIt; subStrIt != subStrings.end(); ++subStrIt)
    {
      // Does this sub-tag exist on this node?
      QTAG_LOG("Current Node: %s\n", node->Tag.c_str());
      QTAG_LOG("\tLooking for %s\n", subStrIt->c_str());
      subTagTreeIt = std::find(subTags->begin(), subTags->end(), *subStrIt);
      if (subTagTreeIt == subTags->end())
      {
        QTAG_LOG("\t%s not seen before, emplacing below %s\n", subStrIt->c_str(), node->Tag.c_str());

        TagTreeNode* parentNode = node;
        // Tag not seen before, add to list
        node = &(subTags->emplace_back(*subStrIt));
        node->ParentTag = parentNode;
        QTAG_LOG("\tSetting %s's parent to be %s\n", node->Tag.c_str(), node->ParentTag->Tag.c_str());
        
        // Fast-path, add rest of sub strings going down from this node
        for (++subStrIt; subStrIt != subStrings.end(); ++subStrIt)
        {
          QTAG_LOG("\tEmplacing %s below %s\n", subStrIt->c_str(), node->Tag.c_str());

          parentNode = node;
          node = &(node->SubTags.emplace_back(*subStrIt));
          node->ParentTag = parentNode;
          QTAG_LOG("\tSetting %s's parent to be %s\n", node->Tag.c_str(), node->ParentTag->Tag.c_str());
        }
        break;
      }
      else
      {
        QTAG_LOG("\t%s seen before, switching to that node\n", subTagTreeIt->Tag.c_str());
        // Update working data
        node = &*subTagTreeIt;
        subTags = &node->SubTags;

        // continue to next substring
        continue;
      }
    }
  }
}

void QTagUtil::EnumerateTags(std::list<TagTreeNode>& tags)
{
  for (int nodeIdx = 0; nodeIdx < tags.size(); ++nodeIdx)
  {
    std::list<TagTreeNode>::iterator it = tags.begin();
    std::advance(it, nodeIdx);
    TagTreeNode& node = *it;
    node.TagAsInt = nodeIdx + 1; // +1 since 0 denotes"None"/"Unset"
    if (node.SubTags.size() > 0)
    {
      EnumerateTags(node.SubTags);
    }
  }
}

void DescendTree(const TagTreeNode& currentNode, const int currentDepth, std::vector<unsigned int>& outRanges)
{
  QTAG_LOG("\tCurrent Node: %s looking at sub tags for depth %d\n", currentNode.Tag.c_str(), currentDepth);

  const std::list<TagTreeNode>& subTags = currentNode.SubTags;
  const int numTagsAtDepth = (int)subTags.size();

  QTAG_LOG("\t\tNum sub tags: %d\n", numTagsAtDepth);
  if (numTagsAtDepth == 0)
  {
    QTAG_LOG("\t\tNo sub tags, returning\n");
    return;
  }

  if (outRanges.size() == currentDepth)
  {
    QTAG_LOG("\t\tNew Depth Entry, setting to %d\n", numTagsAtDepth);
    outRanges.push_back(numTagsAtDepth);
  }
  else
  {
    const int oldEntry = outRanges[currentDepth];
    if (oldEntry < numTagsAtDepth)
    {
      QTAG_LOG("\t\tReplacing entry for depth %d\n", currentDepth);
      outRanges[currentDepth] = numTagsAtDepth;
    }
    else
    {
      QTAG_LOG("\t\tExisting entry for depth %d higher or same (%d vs %d)\n", currentDepth, oldEntry, numTagsAtDepth);
    }
  }

  // Continue to sub nodes
  for (const TagTreeNode& subNode : subTags)
  {
    DescendTree(subNode, currentDepth + 1, outRanges);
  }
}

void QTagUtil::FindTagRanges(const std::list<TagTreeNode>& inTags, std::vector<unsigned int>& outRanges)
{
  outRanges.clear();

  if (inTags.size() == 0)
  {
    return;
  }

  // First range is just the number of trees we have
  outRanges.push_back((unsigned int)inTags.size());
  QTAG_LOG("Num Top-Level Tags: %d\n", outRanges.front());

  // Walk down each tree, finding the biggest TagAsInt for each level, updating outRanges
  for (const TagTreeNode& tree : inTags)
  {
    QTAG_LOG("Tree: %s\n", tree.Tag.c_str()); 
    int depth = 1;
    DescendTree(tree, depth, outRanges);
  }
}

void QTagUtil::GetRequiredBitsPerField(const std::vector<unsigned int>& fieldRanges, std::vector<unsigned int>& outBits)
{
  for (const unsigned int field : fieldRanges)
  {
    outBits.push_back(std::bit_width(field));
  }
}

QTagUtil::EQTagIntBase QTagUtil::FindSmallestIntBase(const std::vector<unsigned int>& inBits)
{
  unsigned int sumOfBits = std::accumulate(inBits.begin(), inBits.end(), 0);
  unsigned int nextPow2 = std::bit_ceil(sumOfBits);

  switch (nextPow2)
  {
  case 8 : return EQTagIntBase::UInt8;
  case 16: return EQTagIntBase::UInt16;
  case 32: return EQTagIntBase::UInt32;
  case 64: return EQTagIntBase::UInt64;
  }
  // If you need more than 64 bits, add your own 128bit case
  return EQTagIntBase::UInt64;
}

std::string QTagUtil::GetTemplateString(const EQTagIntBase base, const std::vector<unsigned int>& fieldBits)
{
  std::stringstream ss;
  ss << "using QTag = QuickTag<";

  switch (base)
  {
  case EQTagIntBase::UInt8 : ss << "std::uint8_t, " ; break;
  case EQTagIntBase::UInt16: ss << "std::uint16_t, "; break;
  case EQTagIntBase::UInt32: ss << "std::uint32_t, "; break;
  default:
  case EQTagIntBase::UInt64: ss << "std::uint64_t, "; break;
  }

  for (size_t f = 0; f < fieldBits.size(); ++f)
  {
    const std::uint32_t fieldSize = fieldBits[f];
    ss << fieldSize;
    if (f < (fieldBits.size() - 1))
    {
      ss << ", ";
    }
  }
  ss << ">;";

  std::string str = ss.str();
  return str;
}

void QTagUtil::Internal::GetAllSubTags(const TagTreeNode& topLevelNode, std::vector<const TagTreeNode*>& outNodes)
{
  for (const TagTreeNode& subTag : topLevelNode.SubTags)
  {
    outNodes.push_back(&subTag);
    GetAllSubTags(subTag, outNodes);
  }
}
