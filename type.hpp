#pragma once
#ifndef MAL_TYPE_HPP
#define MAL_TYPE_HPP

#include <memory>
#include <variant>
#include "hoolib.hpp"

class MalType;
using MalTypePtr = std::shared_ptr<MalType>;

// In env.hpp, MalTypePtr is used.
#include "env.hpp"

class MalFunction;
class MalInteger;
class MalSymbol;
class MalList;

class MalType {
public:
    virtual MalTypePtr eval(EnvPtr env) = 0;
    virtual std::string pr_str() const = 0;
    virtual std::shared_ptr<MalInteger> as_integer() { return nullptr; }
    virtual std::shared_ptr<MalFunction> as_function() { return nullptr; }
    virtual std::shared_ptr<MalSymbol> as_symbol() { return nullptr; }
    virtual std::shared_ptr<MalList> as_list() { return nullptr; }
};

class MalFunction : public MalType,
                    public std::enable_shared_from_this<MalFunction> {
public:
    virtual MalTypePtr call(EnvPtr env,
                            const std::vector<MalTypePtr>& args) = 0;
    MalTypePtr eval(EnvPtr env)
    {
        HOOLIB_THROW("MalFunction couldn't be evaluated");
    }

    std::string pr_str() const { return "<FUNC>"; }
    std::shared_ptr<MalFunction> as_function() { return shared_from_this(); }
};

class MalBuiltInFunction : public MalFunction {
public:
    using Func =
        std::function<MalTypePtr(EnvPtr, const std::vector<MalTypePtr>&)>;

private:
    Func func_;

public:
    MalBuiltInFunction(Func func) : func_(func) {}

    MalTypePtr call(EnvPtr env, const std::vector<MalTypePtr>& args);
};

class MalAtom : public MalType {
};

class MalSymbol : public MalAtom,
                  public std::enable_shared_from_this<MalSymbol> {
private:
    std::string name_;

public:
    MalSymbol(const std::string& name) : name_(name) {}

    std::string pr_str() const { return name_; }
    const std::string& getName() const { return name_; }

    MalTypePtr eval(EnvPtr env);

    std::shared_ptr<MalSymbol> as_symbol() { return shared_from_this(); }
};

class MalInteger : public MalAtom,
                   public std::enable_shared_from_this<MalInteger> {
private:
    long long int data_;

public:
    MalInteger(long long int data) : data_(data) {}

    std::string pr_str() const { return HooLib::to_str(data_); }
    long long int get() const { return data_; }

    MalTypePtr eval(EnvPtr env) { return shared_from_this(); }
    std::shared_ptr<MalInteger> as_integer() { return shared_from_this(); }
};

class MalList : public MalType, public std::enable_shared_from_this<MalList> {
private:
    std::vector<MalTypePtr> list_;

public:
    MalList(std::vector<MalTypePtr> list) : list_(std::move(list)) {}

    std::string pr_str() const
    {
        std::stringstream ss;
        ss << "(";
        for (auto&& item : list_) ss << item->pr_str() << " ";
        ss.seekp(-1, ss.cur);
        ss << ")";
        return ss.str();
    }

    const std::vector<MalTypePtr>& get() const { return list_; }
    std::vector<MalTypePtr>& get() { return list_; }

    MalTypePtr eval(EnvPtr env);
    std::shared_ptr<MalList> as_list() { return shared_from_this(); }
};

#endif
