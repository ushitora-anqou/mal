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
};

#endif
