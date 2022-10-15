#include "QuickTags.hpp"
#include "QuickTags-Loader.hpp"

#include <cstdio>
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
  using namespace QTagUtil;

  std::vector<std::string> tagsFiles;
  bool bCaseInsensitive = false;

  for (int i = 0; i < argc; ++i)
  {
    printf("%s ", argv[i]);

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
        printf("%s", argv[i]);
        tagsFiles.push_back(argv[i]);
      }
      else
      {
        printf("-f param but no file provided");
        return -1;
      }
    } // end -f

    if (arg == "-case-insensitive")
    {
      bCaseInsensitive = true;
    }

    printf("\n");
  }

  if (tagsFiles.empty())
  {
    printf("No file(s) provided");
    return -1;
  }

  std::vector<std::fstream> files;
  files.reserve(tagsFiles.size());
  for (const std::string& tagsFile : tagsFiles)
  {
    printf("Opening file %s for analysis...\n", tagsFile.c_str());

    std::fstream fileStream = std::fstream(tagsFile, std::ios_base::in);
    if (!fileStream.is_open())
    {
      printf("Failed to open file %s", tagsFile.c_str());
      continue;
    }
    files.push_back(std::move(fileStream));
  }

  ETagSetFlags flags = ETagSetFlags::None;
  if (bCaseInsensitive)
  {
    flags = (ETagSetFlags)((unsigned int)flags | (unsigned int)ETagSetFlags::CaseInsensitive);
  }

  std::set<std::string> tagStringSet;
  BuildTagStringSetFromFiles(files, tagStringSet, flags);

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