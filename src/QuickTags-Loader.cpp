#include "QuickTags-Loader.hpp"
#include <sstream>
#include <bit>
#include <numeric>

#include <cstdio>

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

void EnumerateTags(std::vector<TagTreeNode>& tags)
{
  for (int nodeIdx = 0; nodeIdx < tags.size(); ++nodeIdx)
  {
    TagTreeNode& node = tags[nodeIdx];
    node.TagAsInt = nodeIdx + 1; // +1 since 0 denotes"None"/"Unset"
    if (node.SubTags.size() > 0)
    {
      EnumerateTags(node.SubTags);
    }
  }
}

void DescendTree(const TagTreeNode& currentNode, const int currentDepth, std::vector<unsigned int>& outRanges)
{
  printf("\tCurrent Node: %s looking at sub tags for depth %d\n", currentNode.Tag.c_str(), currentDepth);

  const std::vector<TagTreeNode>& subTags = currentNode.SubTags;
  const int numTagsAtDepth = (int)subTags.size();

  printf("\t\tNum sub tags: %d\n", numTagsAtDepth);
  if (numTagsAtDepth == 0)
  {
    printf("\t\tNo sub tags, returning\n");
    return;
  }

  if (outRanges.size() == currentDepth)
  {
    printf("\t\tNew Depth Entry, setting to %d\n", numTagsAtDepth);
    outRanges.push_back(numTagsAtDepth);
  }
  else
  {
    const int oldEntry = outRanges[currentDepth];
    if (oldEntry < numTagsAtDepth)
    {
      printf("\t\tReplacing entry for depth %d\n", currentDepth);
      outRanges[currentDepth] = numTagsAtDepth;
    }
    else
    {
      printf("\t\tExisting entry for depth %d higher or same (%d vs %d)\n", currentDepth, oldEntry, numTagsAtDepth);
    }
  }

  // Continue to sub nodes
  for (const TagTreeNode& subNode : subTags)
  {
    DescendTree(subNode, currentDepth + 1, outRanges);
  }
}

void FindTagRanges(const std::vector<TagTreeNode>& inTags, std::vector<unsigned int>& outRanges)
{
  outRanges.clear();

  if (inTags.size() == 0)
  {
    return;
  }

  // First range is just the number of trees we have
  outRanges.push_back((unsigned int)inTags.size());
  printf("Num Top-Level Tags: %d\n", outRanges.front());

  // Walk down each tree, finding the biggest TagAsInt for each level, updating outRanges
  for (const TagTreeNode& tree : inTags)
  {
    printf("Tree: %s\n", tree.Tag.c_str()); 
    int depth = 1;
    DescendTree(tree, depth, outRanges);
  }
}

void GetRequiredBitsPerField(const std::vector<unsigned int>& fieldRanges, std::vector<unsigned int>& outBits)
{
  for (const unsigned int field : fieldRanges)
  {
    outBits.push_back(std::bit_width(field));
  }
}

EQTagIntBase FindSmallestIntBase(const std::vector<unsigned int>& inBits)
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

std::string GetTemplateString(const EQTagIntBase base, const std::vector<unsigned int>& fieldBits)
{
  //std::string str = ;
  std::stringstream ss;
  ss << "using QTag = QuickTag<";

  switch (base)
  {
  case EQTagIntBase::UInt8 : ss << "std::uint8_t, " ; break;
  case EQTagIntBase::UInt16: ss << "std::uint16_t, "; break;
  case EQTagIntBase::UInt32: ss << "std::uint32_t, "; break;
  case EQTagIntBase::UInt64: ss << "std::uint64_t, "; break;
  }

  //for (const unsigned int fieldSize : fieldBits)
  for (int f = 0; f < fieldBits.size(); ++f)
  {
    const unsigned int fieldSize = fieldBits[f];
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
