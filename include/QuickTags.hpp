#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>

namespace QTagUtil
{
  // Src: https://stackoverflow.com/questions/17719674/c11-fast-constexpr-integer-powers
  inline constexpr int64_t ipow(const int64_t base, int64_t exp, const int64_t result = 1) {
    return exp < 1 ? result : ipow(base * base, exp / 2, (exp % 2) ? result * base : result);
  }
}

template<typename BaseType, unsigned char... Field>
class QuickTag
{
  using TagBaseType = BaseType;
  static constexpr std::size_t NumFields = sizeof...(Field);
  struct NumFieldsSizeArray
  {
    BaseType Data[NumFields];
    constexpr       BaseType& operator[](std::size_t idx)       { return Data[idx]; }
    constexpr const BaseType& operator[](std::size_t idx) const { return Data[idx]; }
  };

public:
  constexpr QuickTag() : Value(0) {}
  explicit constexpr QuickTag(BaseType rawValue) : Value(rawValue) {}
  constexpr QuickTag(const BaseType(&fields)[NumFields])
  {
    Value = 0;
    SetValueFromArray(fields, NumFields);
  }
  constexpr QuickTag(const BaseType* const fields, const int num)
  {
    Value = 0;
    const int loopDepth = num < NumFields ? num : NumFields; // Min
    SetValueFromArray(fields, loopDepth);
  }

  template<class... Args>
  static constexpr QuickTag<BaseType, Field...> MakeTag(Args... fieldValues)
  {
    return QuickTag<BaseType, Field...>({ (BaseType)fieldValues... });
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
    if (Value == 0)
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

  constexpr int GetDepth() const
  {
    if (Value == 0)
    {
      return 0;
    }

    // Check each field until we find a 0
    int depth = 0;
    for (int f = 0; f < NumFields; ++f)
    {
      const BaseType fieldMask = GetMask(f);
      if ((Value & fieldMask) != 0)
      {
        depth++;
      }
      else
      {
        return depth;
      }
    }
    return depth;
  }

  bool operator==(const QuickTag<BaseType, Field...>& rhs) const { return Value == rhs.Value; }
  bool operator!=(const QuickTag<BaseType, Field...>& rhs) const { return !(*this == rhs); }

  bool operator<(const QuickTag<BaseType, Field...>& rhs) const { return Value < rhs.Value; }
  bool operator>(const QuickTag<BaseType, Field...>& rhs) const { return rhs < *this; }
  bool operator<=(const QuickTag<BaseType, Field...>& rhs) const { return !(*this > rhs); }
  bool operator>=(const QuickTag<BaseType, Field...>& rhs) const { return !(*this < rhs); }

  constexpr bool MatchesExact(const QuickTag<BaseType, Field...>& tagToMatch) const
  {
    if (!tagToMatch.IsValid())
    {
      return false;
    }

    return *this == tagToMatch;
  }

  // In the style of FGameplayTag...
  // "A.1".Matches("A") will return True, "A".Matches("A.1") will return False
  constexpr bool Matches(const QuickTag<BaseType, Field...>& tagToMatch) const
  {
    if (!tagToMatch.IsValid())
    {
      return false;
    }

    // Compare depths
    const int myDepth = GetDepth();
    const int theirDepth = tagToMatch.GetDepth();
    if (theirDepth > myDepth)
    {
      return false;
    }
    else if (theirDepth == myDepth)
    {
      return *this == tagToMatch;
    }
    else // theirDepth < myDepth (compare parents)
    {
      // Build mask from their depth
      unsigned int parentMask = 0;
      for (int f = 0; f < theirDepth; ++f)
      {
        parentMask |= GetMask(f);
      }
      // Compare our parents with tagToMatch
      return ((Value & parentMask) == tagToMatch.Value);
    }
  }

  // Allocate a char array with a string containing a textual representation of the Tag's Value
  // string must be deleted/freed by caller!
  char* ValueAsString() const
  {
    unsigned int strLen = 0;
    BaseType fields[NumFields];
    unsigned int fieldSize[NumFields];
    // Calculate how long the string needs to be
    for (int f = 0; f < NumFields; ++f)
    {
      BaseType value = GetField(f);
      fields[f] = value; // Save for string conversion
      
      unsigned int len = 0;
      do
      {
        len += 1; // Always at least 1 ('0')
        value /= 10; // Base 10
      } while (value);

      fieldSize[f] = len;

      if (f < NumFields - 1)
      {
        len += 1; // . separator
      }

      strLen += len;
    }
    strLen += 1; // null terminator

    // Allocate
    //if (char* outStr = (char*)malloc(strLen*sizeof(char)))
    if (char* outStr = new char[strLen])
    {
      char* strPlace = outStr;
      unsigned int remainingStrLen = strLen;
      for (int f = 0; f < NumFields; ++f)
      {
        const BaseType value = fields[f];
        char buf[22]; // 2^64 is 20 characters, +1 for null, +1 for possible minus symbol (unlikely)
        snprintf(buf, 22, "%d", value); // int to string

        // Write buffer to string
        strncpy_s(strPlace, remainingStrLen, buf, fieldSize[f]);
        
        strPlace += fieldSize[f];
        remainingStrLen -= fieldSize[f];

        // Add deliminator
        if (f < NumFields - 1)
        {
          strncpy_s(strPlace, remainingStrLen, ".", 1);
          strPlace++;
          remainingStrLen--;
        }
      }
      // End
      *strPlace = '\0';
      return outStr;
    }
    return nullptr;
  }

private:
  void SetValueFromArray(const BaseType* arr, const int len)
  {
    for (int f = 0; f < len; ++f)
    {
      const BaseType value = arr[f];
      if (value == 0)
      {
        // End of valid data
        return;
      }
      const BaseType offset = GetOffset(f);
      const BaseType mask = GetMask(f);
      Value |= (arr[f] << offset) & mask;
    }
  }

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
