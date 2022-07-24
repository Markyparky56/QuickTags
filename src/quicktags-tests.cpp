#include "QuickTags.hpp"

#include <cstdio>

// 8.8.8.8
using QTag32 = QuickTag<uint32_t, QuickTags::TagBlock<0, 8>, QuickTags::TagBlock<8, 8>, QuickTags::TagBlock<16, 8>, QuickTags::TagBlock<24, 8>>;
// 16.16.16.16
using QTag64 = QuickTag<uint64_t, QuickTags::TagBlock<0, 16>, QuickTags::TagBlock<16, 16>, QuickTags::TagBlock<32, 16>, QuickTags::TagBlock<48, 16>>;
// 10.10.10.2
using QTagVariant0 = QuickTag<uint32_t, QuickTags::TagBlock<0, 10>, QuickTags::TagBlock<10, 10>, QuickTags::TagBlock<20, 10>, QuickTags::TagBlock<30, 2>>;

int main(int argc, char** argv)
{
  //uint32_t rawValue32 = (123u) | (234u << 8) | (99u << 16) | (12u << 24);
  //QTag32 my32BitTag(rawValue32);

  //printf("32-bit Tag Components:\n");
  //printf("Value of A: %u\n", my32BitTag.Get<0>());
  //printf("Value of B: %u\n", my32BitTag.Get<1>());
  //printf("Value of C: %u\n", my32BitTag.Get<2>());
  //printf("Value of D: %u\n", my32BitTag.Get<3>());

  //uint64_t rawValue64 = (4827ull) | (56867ull << 16) | (5334ull << 32) | (30319ull << 48);
  //QTag64 my64BitTag(rawValue64);

  //printf("64-bit Tag Components:\n");
  //printf("Value of A: %llu\n", my64BitTag.Get<0>());
  //printf("Value of B: %llu\n", my64BitTag.Get<1>());
  //printf("Value of C: %llu\n", my64BitTag.Get<2>());
  //printf("Value of D: %llu\n", my64BitTag.Get<3>());

  //uint32_t rawValueVar0 = (744u) | (460u << 10) | (336u << 20) | (1 << 30);
  //QTagVariant0 myVar0Tag(rawValueVar0);

  //printf("32-bit 10.10.10.2 Tag Components:\n");
  //printf("Value of A: %u\n", myVar0Tag.Get<0>());
  //printf("Value of B: %u\n", myVar0Tag.Get<1>());
  //printf("Value of C: %u\n", myVar0Tag.Get<2>());
  //printf("Value of D: %u\n", myVar0Tag.Get<3>());

  //QTagVariant0 myVar0Tag2 = QTagVariant0::MakeTag({ 0,1,2,3 });

  //printf("(Custom Make) 32-bit 10.10.10.2 Tag Components:\n");
  //printf("Value of A: %u\n", myVar0Tag2.Get<0>());
  //printf("Value of B: %u\n", myVar0Tag2.Get<1>());
  //printf("Value of C: %u\n", myVar0Tag2.Get<2>());
  //printf("Value of D: %u\n", myVar0Tag2.Get<3>());

  //QuickTag2<uint64_t, 8, 8, 8, 8, 16, 16> betterTag({ 7,6,5,4,13,10 });
  //printf("64-bit 8.8.8.8.16.16 Tag Components:\n");
  //printf("Value of A: %llu\n", betterTag.GetField(0));
  //printf("Value of B: %llu\n", betterTag.GetField(1));
  //printf("Value of C: %llu\n", betterTag.GetField(2));
  //printf("Value of D: %llu\n", betterTag.GetField(3));
  //printf("Value of E: %llu\n", betterTag.GetField(4));
  //printf("Value of F: %llu\n", betterTag.GetField(5));

  using QTag = QuickTag2<uint32_t, 4, 8, 12, 8>;
  QTag tag = QTag::MakeTag(1, 2, 3);

  return 0;
}