#pragma once
#ifndef MAL_ENV_HPP
#define MAL_ENV_HPP

#include <memory>
#include <unordered_map>

class Env;
using EnvPtr = std::shared_ptr<Env>;

// In type.hpp, EnvPtr is used
#include "type.hpp"

class Env : public std::enable_shared_from_this<Env> {
private:
    std::unordered_map<std::string, MalTypePtr> data_;
    EnvPtr outer_;

public:
    Env(EnvPtr outer = nullptr) : outer_(std::move(outer)) {}

    template <class T>
    Env(EnvPtr outer, const std::vector<std::string>& binds,
        const HooLib::Range<T>& exprs)
        : outer_(std::move(outer))
    {
        HOOLIB_THROW_UNLESS(
            binds.size() == static_cast<decltype(binds.size())>(exprs.size()),
            "invalid argument");
        for (size_t i = 0; i < binds.size(); i++) set(binds[i], exprs[i]);
    }

    void set(const std::string& key, const MalTypePtr& value)
    {
        data_[key] = value;
    }

    EnvPtr find(const std::string& key)
    {
        auto it = data_.find(key);
        if (it != data_.end()) return shared_from_this();
        HOOLIB_THROW_IF(outer_ == nullptr,
                        HooLib::fok("key '", key, "' was not found."));
        return outer_->find(key);
    }

    MalTypePtr get(const std::string& key) { return find(key)->data_[key]; }
    MalTypePtr get_if(const std::string& key)
    {
        try {
            return get(key);
        }
        catch (...) {
            return nullptr;
        }
    }
};

#endif
