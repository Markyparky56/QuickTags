#include "QuickTags.hpp"
#include "QuickTags-Loader.hpp"

#include <cstdio>

int main(int argc, char** argv)
{
  using QTag = QuickTag<uint32_t, 4, 8, 12, 8>;
  QTag tag = QTag::MakeTag(1, 2, 3);
  QTag tag2 = QTag::MakeTag(1, 2);

  char* tagAsString = tag.ValueAsString();
  printf("tag as string:\t%s\n", tagAsString);
  delete[] tagAsString;

  char* tag2AsString = tag2.ValueAsString();
  printf("tag2 as string:\t%s\n", tag2AsString);
  delete[] tag2AsString;

  printf("tag.Matches(tag2): %d\n", tag.Matches(tag2));
  printf("tag2.Matches(tag): %d\n", tag2.Matches(tag));

  using QTag2 = QuickTag<uint8_t, 2, 2, 2, 1, 1>;

  std::fstream file = std::fstream("../../../../src/Tags.txt", std::ios_base::in);
  if (!file.is_open())
  {
    return -1;
  }

  std::map<QTag2, std::string> tagStringMap;
  std::vector<QTag2> tags;
  QTagUtil::LoadQuickTagsFromFile(file, tagStringMap, tags);

  for (const std::pair<QTag2, std::string>& tag : tagStringMap)
  {
    char* tagString = tag.first.ValueAsString();
    printf("%s:\t%s\n", tagString, tag.second.c_str());
    delete[] tagString;
  }

  for (const QTag2& tag : tags)
  {
    printf("%d\n", tag.GetRaw());
  }

  return 0;
}