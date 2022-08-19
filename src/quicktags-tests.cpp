#include "QuickTags.hpp"

#include <cstdio>

int main(int argc, char** argv)
{
  using QTag = QuickTag<uint32_t, 4, 8, 12, 8>;
  QTag tag = QTag::MakeTag(1, 2, 3);
  QTag tag2 = QTag::MakeTag(1, 2);

  char* tagAsString = tag.ValueAsString();
  printf("tag as string: %s\n", tagAsString);
  delete[] tagAsString;

  printf("tag.Matches(tag2): %d\n", tag.Matches(tag2));
  printf("tag2.Matches(tag): %d\n", tag2.Matches(tag));

  return 0;
}