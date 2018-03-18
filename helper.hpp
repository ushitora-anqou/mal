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
    return ":" + std::string(keyword.begin() + 1, keyword.end());
}

inline bool is_pair(const MalTypePtr& value)
{
    const auto seq = value->as_sequential();
    if (!seq) return false;
    return !seq->get().empty();
}

template <class Iterator, class Callback>
void each_odd_even_pair(Iterator begin, Iterator end, Callback cb)
{
    using std::advance;

    auto first = begin, second = begin;
    ++second;
    while (first != end) {
        HOOLIB_THROW_UNLESS(second != end, "invalid argument");
        cb(*first, *second);
        advance(first, 2);
        advance(second, 2);
    }
}

template <class Iterator>
void insert_odd_even_list(MalHashMap::Container& cont, Iterator begin,
                          Iterator end)
{
    each_odd_even_pair(begin, end, [&cont](auto&& first, auto&& second) {
        auto key = first->as_string();
        HOOLIB_THROW_UNLESS(key, "invalid argument");
        cont[key->get()] = second;
    });
}

// input: odd-even list
template <class Iterator>
MalHashMap::Container make_hash_map_container(Iterator begin, Iterator end)
{
    MalHashMap::Container cont;
    insert_odd_even_list(cont, begin, end);
    return cont;
}

}  // namespace mal::helper

#endif
