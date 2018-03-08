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
class MalNil;
class MalTrue;
class MalFalse;
class MalString;
class MalSequential;
class MalList;
class MalVector;

class MalType : public std::enable_shared_from_this<MalType> {
public:
    virtual MalTypePtr eval(EnvPtr env) = 0;

    virtual std::string pr_str(bool print_readably) const = 0;

    virtual std::shared_ptr<MalInteger> as_integer() { return nullptr; }
    virtual std::shared_ptr<const MalInteger> as_integer() const
    {
        return nullptr;
    }
    virtual std::shared_ptr<MalFunction> as_function() { return nullptr; }
    virtual std::shared_ptr<const MalFunction> as_function() const
    {
        return nullptr;
    }
    virtual std::shared_ptr<MalSymbol> as_symbol() { return nullptr; }
    virtual std::shared_ptr<const MalSymbol> as_symbol() const
    {
        return nullptr;
    }
    virtual std::shared_ptr<MalNil> as_nil() { return nullptr; }
    virtual std::shared_ptr<const MalNil> as_nil() const { return nullptr; }
    virtual std::shared_ptr<MalTrue> as_true() { return nullptr; }
    virtual std::shared_ptr<const MalTrue> as_true() const { return nullptr; }
    virtual std::shared_ptr<MalFalse> as_false() { return nullptr; }
    virtual std::shared_ptr<const MalFalse> as_false() const { return nullptr; }
    virtual std::shared_ptr<MalString> as_string() { return nullptr; }
    virtual std::shared_ptr<const MalString> as_string() const
    {
        return nullptr;
    }
    virtual std::shared_ptr<MalSequential> as_sequential() { return nullptr; }
    virtual std::shared_ptr<const MalSequential> as_sequential() const
    {
        return nullptr;
    }
    virtual std::shared_ptr<MalList> as_list() { return nullptr; }
    virtual std::shared_ptr<const MalList> as_list() const { return nullptr; }
    virtual std::shared_ptr<MalVector> as_vector() { return nullptr; }
    virtual std::shared_ptr<const MalVector> as_vector() const
    {
        return nullptr;
    }

    virtual bool is_equal_to(const MalTypePtr& rhs) const
    {
        return shared_from_this() == rhs;
    }
};

// helper macros to make classes derived from MalType
#define MAL_DEFINE_GET_THIS_PTR(classname)                                    \
private:                                                                      \
    std::shared_ptr<classname> get_this_pointer()                             \
    {                                                                         \
        return std::static_pointer_cast<classname>(shared_from_this());       \
    }                                                                         \
    std::shared_ptr<const classname> get_this_pointer() const                 \
    {                                                                         \
        return std::static_pointer_cast<const classname>(shared_from_this()); \
    }

#define MAL_DEFINE_AS(classname, typename)                          \
public:                                                             \
    std::shared_ptr<classname> as_##typename() override             \
    {                                                               \
        return get_this_pointer();                                  \
    }                                                               \
    std::shared_ptr<const classname> as_##typename() const override \
    {                                                               \
        return get_this_pointer();                                  \
    }

class MalFunction : public MalType {
    MAL_DEFINE_GET_THIS_PTR(MalFunction);
    MAL_DEFINE_AS(MalFunction, function);

public:
    using Args = HooLib::Range<std::vector<MalTypePtr>::const_iterator>;
    using Func = std::function<MalTypePtr(const Args&)>;

private:
    Func func_;

public:
    MalFunction(Func func) : func_(func) {}
    MalTypePtr call(const Args& args) { return func_(args); }
    MalTypePtr eval(EnvPtr env)
    {
        HOOLIB_THROW("MalFunction couldn't be evaluated");
    }

    std::string pr_str(bool print_readably) const { return "#<function>"; }
};

class MalAtom : public MalType {
};

class MalSymbol : public MalAtom {
    MAL_DEFINE_GET_THIS_PTR(MalSymbol);
    MAL_DEFINE_AS(MalSymbol, symbol);

private:
    std::string name_;

public:
    MalSymbol(const std::string& name) : name_(name) {}

    std::string pr_str(bool print_readably) const { return name_; }
    const std::string& name() const { return name_; }

    MalTypePtr eval(EnvPtr env);

    bool is_equal_to(const MalTypePtr& rhs) const
    {
        auto r = rhs->as_symbol();
        return r && r->name() == name_;
    }
};

class MalInteger : public MalAtom {
    MAL_DEFINE_GET_THIS_PTR(MalInteger);
    MAL_DEFINE_AS(MalInteger, integer);

private:
    long long int data_;

public:
    MalInteger(long long int data) : data_(data) {}

    std::string pr_str(bool print_readably) const
    {
        return HooLib::to_str(data_);
    }
    long long int get() const { return data_; }

    MalTypePtr eval(EnvPtr env) { return get_this_pointer(); }
    bool is_equal_to(const MalTypePtr& rhs) const
    {
        auto r = rhs->as_integer();
        return r && r->get() == data_;
    }
};

class MalNil : public MalAtom {
    MAL_DEFINE_GET_THIS_PTR(MalNil);
    MAL_DEFINE_AS(MalNil, nil);

public:
    std::string pr_str(bool print_readably) const { return "nil"; }
    MalTypePtr eval(EnvPtr env) { return get_this_pointer(); }
};

class MalTrue : public MalAtom {
    MAL_DEFINE_GET_THIS_PTR(MalTrue);
    MAL_DEFINE_AS(MalTrue, true);

public:
    std::string pr_str(bool print_readably) const { return "true"; }
    MalTypePtr eval(EnvPtr env) { return get_this_pointer(); }
};

class MalFalse : public MalAtom {
    MAL_DEFINE_GET_THIS_PTR(MalFalse);
    MAL_DEFINE_AS(MalFalse, false);

public:
    std::string pr_str(bool print_readably) const { return "false"; }
    MalTypePtr eval(EnvPtr env) { return get_this_pointer(); }
};

class MalString : public MalAtom {
    MAL_DEFINE_GET_THIS_PTR(MalString);
    MAL_DEFINE_AS(MalString, string);

private:
    std::string data_;

public:
    MalString(const std::string& data) : data_(data) {}

    const std::string& get() const { return data_; }

    std::string pr_str(bool print_readably) const;

    MalTypePtr eval(EnvPtr env) { return get_this_pointer(); }

    bool is_equal_to(const MalTypePtr& rhs) const
    {
        auto r = rhs->as_string();
        return r && data_ == r->data_;
    }
};

class MalSequential : public MalType {
    MAL_DEFINE_GET_THIS_PTR(MalSequential);
    MAL_DEFINE_AS(MalSequential, sequential);

private:
    std::vector<MalTypePtr> items_;

protected:
    std::vector<MalTypePtr> eval_items(EnvPtr env);

public:
    MalSequential(std::vector<MalTypePtr> items) : items_(std::move(items)) {}

    virtual std::string pr_str(bool print_readably) const
    {
        return HooLib::join(HOOLIB_RANGE(items_), " ",
                            [print_readably](auto item) {
                                return item->pr_str(print_readably);
                            });
    }

    const std::vector<MalTypePtr>& get() const { return items_; }
    std::vector<MalTypePtr>& get() { return items_; }

    bool is_equal_to(const MalTypePtr& rhs) const;
};

class MalList : public MalSequential {
    MAL_DEFINE_GET_THIS_PTR(MalList);
    MAL_DEFINE_AS(MalList, list);

public:
    MalList(std::vector<MalTypePtr> items) : MalSequential(std::move(items)) {}

    std::string pr_str(bool print_readably) const
    {
        std::stringstream ss;
        ss << "(" << MalSequential::pr_str(print_readably) << ")";
        return ss.str();
    }

    MalTypePtr eval(EnvPtr env);
};

class MalVector : public MalSequential {
    MAL_DEFINE_GET_THIS_PTR(MalVector);
    MAL_DEFINE_AS(MalVector, vector);

public:
    MalVector(std::vector<MalTypePtr> items) : MalSequential(std::move(items))
    {
    }

    std::string pr_str(bool print_readably) const
    {
        std::stringstream ss;
        ss << "[" << MalSequential::pr_str(print_readably) << "]";
        return ss.str();
    }

    MalTypePtr eval(EnvPtr env);
};

MalTypePtr mal_eval(MalTypePtr ast, EnvPtr env);

#endif
