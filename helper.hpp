#pragma once
#ifndef MAL_HELPER_HPP
#define MAL_HELPER_HPP

#include <iostream>
#include "type.hpp"

namespace mal::helper {

inline std::string string2keyword(const std::string& token)
{
    static const char prefix[2] = {(char)0xff, '\0'};
    return prefix + token;
}

inline bool is_keyword(const std::string& src)
{
    return src.size() >= 1 && src[0] == (char)0xff;
}

inline std::string keyword2string(const std::string& keyword)
{
    return std::string(keyword.begin() + 1, keyword.end());
}

}  // namespace mal::helper

#endif
