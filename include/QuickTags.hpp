#pragma once
#include <stdint.h>
#include <tuple>

namespace QTagUtil
{
  // Src: https://stackoverflow.com/questions/17719674/c11-fast-constexpr-integer-powers
  inline constexpr int64_t ipow(const int64_t base, int64_t exp, const int64_t result = 1) {
    return exp < 1 ? result : ipow(base * base, exp / 2, (exp % 2) ? result * base : result);
  }
}

namespace QuickTags
{
  // Helper to expose ::Type for expansion in std::tuple
  template<typename OuterType>
  class TagBlockBaseType
  {
  public:
    using Type = OuterType;
  };

  template<int offset, int maskSize>
  class TagBlock : public TagBlockBaseType<TagBlock<offset, maskSize>>
  {
    static_assert(maskSize != 0, "Invalid MaskSize for TagBlock");
    static_assert(offset >= 0 && offset < 63, "Invalid Offset for TagBlock");
  public:
    // How many bits to shift to get the value
    static constexpr uint8_t Offset = offset;
    // How large is the mask
    static constexpr uint8_t MaskSize = maskSize;
  };
}

template<typename BaseType, class... TagBlocks>
class QuickTag
{
  using FieldsType = std::tuple<typename TagBlocks::Type...>;
public:
  QuickTag() : QuickTag(0) {}
  QuickTag(BaseType value)
    : Value(value)
  {
  }

  template<std::size_t N>
  static QuickTag<BaseType, TagBlocks...> MakeTag(const BaseType(&comps)[N])
  {
    static_assert(N == std::tuple_size_v<FieldsType>, "MakeTag called with incorrect number of components");
    QuickTag tag;
    tag.SetComponents<N>(comps, std::make_integer_sequence<int, N>{});
    return tag;
  }

  template<int FieldN>
  BaseType Get() const
  {
    constexpr auto field = std::get<FieldN>(Fields);
    return (Value & GetFieldMask<FieldN>()) >> field.Offset;
  }

private:
  template<int FieldN>
  constexpr BaseType GetFieldMask() const
  {
    constexpr auto field = std::get<FieldN>(Fields);
    constexpr uint8_t offset = field.Offset;
    constexpr uint8_t maskSize = field.MaskSize;
    constexpr BaseType mask = BaseType(QTagUtil::ipow(2, maskSize) - 1) << offset;
    return mask;
  }

  template<int Size, int... Ns>
  void SetComponents(const BaseType(&comps)[Size], std::integer_sequence<int, Ns...>&&)
  {
    SetComponents<Size, Ns...>(comps);
  }

  template<int Size>
  void SetComponents(const BaseType(&comps)[Size])
  {
  }

  template<int Size, int N, int... Ns>
  void SetComponents(const BaseType(&comps)[Size])
  {
    constexpr auto field = std::get<N>(Fields);    
    Value |= (comps[N] << field.Offset) & GetFieldMask<N>();

    SetComponents<Size, Ns...>(comps);
  }


  static constexpr FieldsType Fields = std::make_tuple(TagBlocks()...);
  BaseType Value;
};

template<typename BaseType, unsigned char... Blocks>
class QuickTag2
{
  static constexpr std::size_t NumBlocks = sizeof...(Blocks);
  static constexpr unsigned char Blocks[NumBlocks] = { Blocks... };

public:


private:
  BaseType Value;
};

/*
* Untemplated Reference
class QuickTag
{
public:
  template<int offset, int maskSize>
  struct TagBlock
  {
    static_assert(maskSize != 0, "Invalid MaskSize for TagBlock");
    static_assert(offset >= 0 && offset < 63, "Invalid Offset for TagBlock");

    // How many bits to shift to get the value
    static constexpr uint8_t Offset = offset;
    // How large is the mask
    static constexpr uint8_t MaskSize = maskSize;
  };

  QuickTag() : QuickTag(0) {}
  QuickTag(uint64_t value)
    : Value(value)
  {
  }

  template<int FieldN>
  uint64_t Get()
  {
    constexpr TagBlock field = std::get<FieldN>(Fields);
    constexpr uint64_t mask = uint64_t(QTagUtil::ipow(2, field.MaskSize) - 1) << field.Offset;
    return (Value & mask) >> field.Offset;
  }

private:
  static constexpr std::tuple<TagBlock<0, 16>, TagBlock<16, 16>, TagBlock<32, 16>, TagBlock<48, 16>> Fields
    = std::make_tuple(TagBlock<0, 16>(), TagBlock<16, 16>(), TagBlock<32, 16>(), TagBlock<48, 16>());
  uint64_t Value;
};
*/
