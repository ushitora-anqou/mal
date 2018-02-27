#pragma once
#ifndef MAL_TYPE_HPP
#define MAL_TYPE_HPP

#include <memory>
#include <variant>
#include "hoolib.hpp"

class MalType {
public:
    virtual std::string pr_str() const = 0;
};
using MalTypePtr = std::shared_ptr<MalType>;

class MalAtom : public MalType {
};

class MalSymbol : public MalAtom {
private:
    std::string name_;

public:
    MalSymbol(const std::string& name) : name_(name) {}

    std::string pr_str() const { return name_; }
    const std::string& getName() const { return name_; }
};

class MalBuiltInType : public MalAtom {
    using Variable = std::variant<long long int>;

private:
    Variable data_;

public:
    MalBuiltInType(Variable data) : data_(data) {}

    std::string pr_str() const
    {
        return std::visit([](auto&& d) { return HooLib::to_str(d); }, data_);
    }

    const Variable& get() const { return data_; }
    template <class T>
    const T&& get() const
    {
        return std::get<T>(data_);
    }
};

class MalList : public MalType {
private:
    std::vector<MalTypePtr> list_;

public:
    MalList(std::vector<MalTypePtr> list) : list_(list) {}

    std::string pr_str() const
    {
        std::stringstream ss;
        ss << "(";
        for (auto&& item : list_) ss << item->pr_str() << " ";
        ss.seekp(-1, ss.cur);
        ss << ")";
        return ss.str();
    }
};

#endif
