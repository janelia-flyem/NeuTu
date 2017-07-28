#ifndef ZFLAGS_H
#define ZFLAGS_H

#include <type_traits>

template<typename TEnum>
struct is_flags : public std::false_type
{
};

// usage:
#define DECLARE_OPERATORS_FOR_ENUM(TEnum) \
  template <> \
  struct is_flags<TEnum> : public std::true_type \
  { \
    static_assert(std::is_enum<TEnum>::value, "TEnum must be enum type"); \
  };

// in global namespace so it doesn't hide qt's operator
//http://stackoverflow.com/questions/10755058/qflags-enum-type-conversion-fails-all-of-a-sudden
// impl:
#define INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_UNARY_OPERATOR(OP) \
  template<typename TEnum> \
  constexpr typename std::enable_if<is_flags<TEnum>::value, TEnum>::type \
  operator OP(TEnum value) noexcept \
  { \
    using underlyingT = typename std::underlying_type<TEnum>::type; \
    return static_cast<TEnum>(OP static_cast<underlyingT>(value)); \
  }

#define INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_BINARY_OPERATOR(OP) \
  template<typename TEnum> \
  constexpr typename std::enable_if<is_flags<TEnum>::value, TEnum>::type \
  operator OP(TEnum l, TEnum r) noexcept \
  { \
    using underlyingT = typename std::underlying_type<TEnum>::type; \
    return static_cast<TEnum>(static_cast<underlyingT>(l) OP static_cast<underlyingT>(r)); \
  } \
  template<typename TEnum> \
  constexpr typename std::enable_if<is_flags<TEnum>::value, TEnum&>::type \
  operator OP##=(TEnum& l, TEnum r) noexcept \
  { \
    return l = l OP r; \
  }

INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_UNARY_OPERATOR(~)
INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_BINARY_OPERATOR(|)
INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_BINARY_OPERATOR(&)
INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_BINARY_OPERATOR(^)

#undef INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_UNARY_OPERATOR
#undef INTERNAL_IMPLEMENTATION_DETAIL_DO_NOT_USE_DECLARE_ENUM_BINARY_OPERATOR

template<typename TEnum, typename std::enable_if<is_flags<TEnum>::value, int>::type = 0>
constexpr bool is_flag_set(TEnum value, TEnum flag) noexcept
{
  using underlyingT = typename std::underlying_type<TEnum>::type;
  return (static_cast<underlyingT>(value) & static_cast<underlyingT>(flag)) != 0;
}

template<typename TEnum, typename std::enable_if<is_flags<TEnum>::value, int>::type = 0>
/*constexpr*/ void set_flag(TEnum& value) noexcept
{
  value |= ~value;
}

template<typename TEnum, typename std::enable_if<is_flags<TEnum>::value, int>::type = 0>
/*constexpr*/ void set_flag(TEnum& value, TEnum flag) noexcept
{
  value |= flag;
}

template<typename TEnum, typename std::enable_if<is_flags<TEnum>::value, int>::type = 0>
/*constexpr*/ void reset_flag(TEnum& value) noexcept
{
  value &= ~value;
}

template<typename TEnum, typename std::enable_if<is_flags<TEnum>::value, int>::type = 0>
/*constexpr*/ void reset_flag(TEnum& value, TEnum flag) noexcept
{
  value &= ~flag;
}

template<typename TEnum, typename std::enable_if<is_flags<TEnum>::value, int>::type = 0>
/*constexpr*/ void flip_flag(TEnum& value) noexcept
{
  value = ~value;
}

template<typename TEnum, typename std::enable_if<is_flags<TEnum>::value, int>::type = 0>
/*constexpr*/ void flip_flag(TEnum& value, TEnum flag) noexcept
{
  value ^= flag;
}

#endif // ZFLAGS_H
