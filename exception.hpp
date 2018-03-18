#pragma once
#ifndef MAL_EXCEPTION_HPP
#define MAL_EXCEPTION_HPP

#include "factory.hpp"
#include "hoolib.hpp"

namespace mal {
class Exception {
private:
    MalTypePtr value_;

public:
    Exception(MalTypePtr value) : value_(value) {}

    MalTypePtr get() { return value_; }
    const MalTypePtr get() const { return value_; }
};

#define MAL_THROW(value) throw mal::Exception(value);
#define MAL_THROW_STRING(...) MAL_THROW(mal::string(HooLib::fok(__VA_ARGS__)));

}  // namespace mal

#endif
