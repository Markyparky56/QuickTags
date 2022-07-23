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

template<typename BaseType, unsigned char... Field>
class QuickTag2
{
  static constexpr std::size_t NumFields = sizeof...(Field);
  struct NumFieldsSizeArray
  {
    BaseType Data[NumFields];
    constexpr       BaseType& operator[](std::size_t idx)       { return Data[idx]; }
    constexpr const BaseType& operator[](std::size_t idx) const { return Data[idx]; }
  };

public:
  constexpr QuickTag2() : Value(0) {}
  explicit constexpr QuickTag2(BaseType rawValue) : Value(rawValue) {}
  constexpr QuickTag2(const BaseType(&fields)[NumFields])
  {
    Value = 0;
    for (int f = 0; f < NumFields; ++f)
    {
      const BaseType value = fields[f];
      if (value == 0)
      {
        // End of valid data
        return;
      }
      const BaseType offset = GetOffset(f);
      const BaseType mask = GetMask(f);
      Value |= (fields[f] << offset) & mask;
    }
  }

  // Non-templated GetField when index is not known at compile time
  constexpr BaseType GetField(const unsigned char field) const
  {
    return (Value & GetMask(field)) >> GetOffset(field);
  }

  // Templated GetField when index is known. GCC prefers this to the above variant.
  template<unsigned char field>
  constexpr BaseType GetField() const
  {
    return GetField(field);
  }

  constexpr void SetField(const unsigned char field, const BaseType fieldValue)
  {
    const BaseType maskedValue = (fieldValue & GetMaskSize(field)) << GetOffset(field);
    // (Value & ~maskValue) unsets old field
    // | maskValue) sets to new field value
    Value = (Value & ~maskedValue) | maskedValue;
  }

  template<unsigned char field>
  constexpr BaseType SetField(const BaseType fieldValue) const
  {
    SetField(field, fieldValue);
  }

  constexpr bool IsValid() const
  {
    // First check if Value is set
    if (Value != 0)
    {
      return false;
    }

    // Then scan backwards from last field, if we find a null value before a valid value, we've got bad data
    bool bPrevFieldWasValid = false;
    for (int f = NumFields-1; f >= 0; --f)
    {
      const BaseType field = Value & GetMask(f);
      if (field != 0)
      {
        bPrevFieldWasValid |= true;
      }
      else if(bPrevFieldWasValid)
      {
        return false;
      }
    }
    return true;
  }

  //constexpr int GetDepth() const
  //{
  //  // Check each field until we find a 0
  //  int depth = 0;
  //}

  bool operator==(const QuickTag2<BaseType, Field...>& rhs) const { return Value == rhs.Value; }
  bool operator!=(const QuickTag2<BaseType, Field...>& rhs) const { return !(*this == rhs); }

  bool operator<(const QuickTag2<BaseType, Field...>& rhs) const { return Value < rhs.Value; }
  bool operator>(const QuickTag2<BaseType, Field...>& rhs) const { return rhs < *this; }
  bool operator<=(const QuickTag2<BaseType, Field...>& rhs) const { return !(*this > rhs); }
  bool operator>=(const QuickTag2<BaseType, Field...>& rhs) const { return !(*this < rhs); }

  constexpr bool MatchesExact(const QuickTag2<BaseType, Field...>& tagToMatch) const
  {
    return *this == tagToMatch;
  }

private:
  static constexpr NumFieldsSizeArray GenFieldOffsets()
  {
    constexpr unsigned char bits = sizeof(BaseType) * 8;
    NumFieldsSizeArray fieldOffsets = { 0 };
    for (int f = 0; f < NumFields; ++f)
    {
      // Pack backwards so that first field is "largest"
      // This lets us sort with <
      unsigned char offset = Fields[0];
      for (int nf = 1; nf < f + 1; ++nf)
      {
        offset += Fields[nf];
      }
      fieldOffsets[f] = bits - offset;
    }
    return fieldOffsets;
  }

  static constexpr NumFieldsSizeArray GenFieldMaskSizes()
  {
    NumFieldsSizeArray fieldMaskSizes = { 0 };
    for (int f = 0; f < NumFields; ++f)
    {
      fieldMaskSizes[f] = BaseType(QTagUtil::ipow(2, Fields[f]) - 1);
    }
    return fieldMaskSizes;
  }

  static constexpr NumFieldsSizeArray GenFieldMasks()
  {
    NumFieldsSizeArray fieldMasks = { 0 };
    for (int f = 0; f < NumFields; ++f)
    {
      fieldMasks[f] = GetMaskSize(f) << GetOffset(f);
    }
    return fieldMasks;
  }

  static constexpr BaseType GetOffset(const unsigned char field)
  {
    return FieldOffsets[field];
  }
  static constexpr BaseType GetMaskSize(const unsigned char field)
  {
    return FieldMaskSizes[field];
  }
  static constexpr BaseType GetMask(const unsigned char field)
  {
    return FieldMasks[field];
  }

  BaseType Value;

  static constexpr unsigned char Fields[NumFields] = { Field... };
  static constexpr NumFieldsSizeArray FieldOffsets = GenFieldOffsets();
  static constexpr NumFieldsSizeArray FieldMaskSizes = GenFieldMaskSizes();
  static constexpr NumFieldsSizeArray FieldMasks = GenFieldMasks();
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
