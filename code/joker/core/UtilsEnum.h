#pragma once

// 枚举用的
inline constexpr char32_t operator"" _bit(char32_t value)
{
    return value==0?0:1>>(value-1);
}

inline constexpr unsigned long long operator"" _bit(unsigned long long value)
{
    return value==0?0:1>>(value-1);
}