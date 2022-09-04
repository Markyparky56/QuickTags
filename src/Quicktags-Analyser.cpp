#include "QuickTags.hpp"
#include "QuickTags-Loader.hpp"

#include <cstdio>
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
  using namespace QTagUtil;

  std::string tagsFile;

  for (int i = 0; i < argc; ++i)
  {
    printf("%s\n", argv[i]);
    std::string arg = argv[i];
    if (arg == "-f")
    {
      if (i + 1 < argc)
      {
        ++i;
        std::string nextArg = argv[i];
        if (char firstChar = *nextArg.begin())
        {
          if (firstChar == '-')
          {
            printf("TagsFile param missing or invalid (%s)", nextArg.c_str());
            return -1;
          }
        }
        tagsFile = argv[i];
      }
      else
      {
        printf("-f param but no file provided");
        return -1;
      }
    } // end -f    
  }

  if (tagsFile.empty())
  {
    printf("No file provided");
    return -1;
  }

  printf("Analysing %s ...\n", tagsFile.c_str());

  std::fstream fileStream = std::fstream(tagsFile, std::ios_base::in);
  if (!fileStream.is_open())
  {
    printf("Failed to open file %s", tagsFile.c_str());
    return -2;
  }

  std::set<std::string> tagStringSet;
  BuildTagStringSetFromFile(fileStream, tagStringSet);

  if (tagStringSet.size() == 0)
  {
    printf("No valid tags found in file");
    return -3;
  }

  // Verbose
  for (const std::string& tagString : tagStringSet)
  {
    printf("Found Tag %s\n", tagString.c_str());
  }

  // Build tree of tags
  std::list<TagTreeNode> tagTrees;
  TreeifyTags(tagStringSet, tagTrees);

  // Enumerate (not necessary for analysis)
  EnumerateTags(tagTrees);

  std::vector<unsigned int> ranges;
  FindTagRanges(tagTrees, ranges);

  std::vector<unsigned int> requiredBitsPerField;
  GetRequiredBitsPerField(ranges, requiredBitsPerField);

  // Output template configuration
  std::string usingString = GetTemplateString(FindSmallestIntBase(requiredBitsPerField), requiredBitsPerField);
  printf("Recommended QTag Configuration:\n%s\n", usingString.c_str());

  return 0;
}