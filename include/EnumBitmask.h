////////////////////////////////////////////////////////////////
//
//	EnumBitmask
//	https://github.com/Reputeless/EnumBitmask
//	License: CC0 1.0 Universal
//
#pragma once

#include <type_traits>

namespace EnumBitmask {

template <typename ENUM>
    requires std::is_enum_v<ENUM>
[[nodiscard]] inline constexpr auto contains(const ENUM &lhs,
                                             const ENUM &rhs) noexcept {
    using U = std::underlying_type_t<ENUM>;
    return (static_cast<U>(lhs) & static_cast<U>(rhs)) == static_cast<U>(rhs);
}

template <typename ENUM>
    requires std::is_enum_v<ENUM>
[[nodiscard]] inline constexpr auto operator&(const ENUM &lhs,
                                              const ENUM &rhs) noexcept {
    using U = std::underlying_type_t<ENUM>;
    return ENUM(static_cast<U>(lhs) & static_cast<U>(rhs));
}

template <typename ENUM>
    requires std::is_enum_v<ENUM>
[[nodiscard]] inline constexpr auto operator|(const ENUM &lhs,
                                              const ENUM &rhs) noexcept {
    using U = std::underlying_type_t<ENUM>;
    return ENUM(static_cast<U>(lhs) | static_cast<U>(rhs));
}

template <typename ENUM>
    requires std::is_enum_v<ENUM>
[[nodiscard]] inline constexpr auto operator^(const ENUM &lhs,
                                              const ENUM& rhs) noexcept {
    using U = std::underlying_type_t<ENUM>;
    return ENUM(static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

template <typename ENUM>
    requires std::is_enum_v<ENUM>
[[nodiscard]] inline constexpr ENUM operator~(const ENUM &value) noexcept {
    using U = std::underlying_type_t<ENUM>;
    return ENUM(~static_cast<U>(value));
}

template <typename ENUM>
    requires std::is_enum_v<ENUM>
inline constexpr ENUM &operator&=(ENUM &lhs, const ENUM &rhs) noexcept {
    return lhs = (lhs & rhs);
}

template <typename ENUM>
    requires std::is_enum_v<ENUM>
inline constexpr ENUM &operator|=(ENUM &lhs, const ENUM &rhs) noexcept {
    return lhs = (lhs | rhs);
}

template <typename ENUM>
    requires std::is_enum_v<ENUM>
inline constexpr ENUM &operator^=(ENUM &lhs, const ENUM &rhs) noexcept {
    return lhs = (lhs ^ rhs);
}

} // namespace EnumBitmask

////////////////////////////////////////////////////////////////