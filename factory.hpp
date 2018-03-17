#pragma once
#ifndef MAL_FACTORY_HPP
#define MAL_FACTORY_HPP

#include "type.hpp"

namespace mal {
namespace detail {
template <class T, class... Args>
std::shared_ptr<T> make_shared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
}  // namespace detail
template <class T, class... Args>
std::shared_ptr<T> make_shared(Args&&... args)
{
    return detail::make_shared<T>(std::forward<Args>(args)...);
}

template <>
inline std::shared_ptr<MalNil> make_shared()
{
    static std::shared_ptr<MalNil> instance = detail::make_shared<MalNil>();
    return instance;
}

template <>
inline std::shared_ptr<MalTrue> make_shared()
{
    static std::shared_ptr<MalTrue> instance = detail::make_shared<MalTrue>();
    return instance;
}

template <>
inline std::shared_ptr<MalFalse> make_shared()
{
    static std::shared_ptr<MalFalse> instance = detail::make_shared<MalFalse>();
    return instance;
}
inline std::shared_ptr<MalList> list() { return ::mal::make_shared<MalList>(); }
inline std::shared_ptr<MalList> list(const std::vector<MalTypePtr>& items)
{
    return ::mal::make_shared<MalList>(items);
}
inline std::shared_ptr<MalSymbol> symbol(const std::string& name)
{
    return ::mal::make_shared<MalSymbol>(name);
}
inline std::shared_ptr<MalInteger> int_(long long int num)
{
    return make_shared<MalInteger>(num);
}
inline std::shared_ptr<MalAtom> atom(const MalTypePtr& ref)
{
    return ::mal::make_shared<MalAtom>(ref);
}
inline std::shared_ptr<MalString> string(const std::string& str)
{
    return ::mal::make_shared<MalString>(str);
}
inline std::shared_ptr<MalNil> nil() { return make_shared<MalNil>(); }
inline std::shared_ptr<MalTrue> true_() { return make_shared<MalTrue>(); }
inline std::shared_ptr<MalFalse> false_() { return make_shared<MalFalse>(); }
inline MalTypePtr boolean(bool b)
{
    if (b)
        return true_();
    else
        return false_();
}

}  // namespace mal

#endif
