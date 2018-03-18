#include "env.hpp"
#include "exception.hpp"

EnvPtr Env::find(const std::string& key)
{
    auto it = data_.find(key);
    if (it != data_.end()) return shared_from_this();
    if (outer_ == nullptr) MAL_THROW_STRING("'", key, "' not found");
    return outer_->find(key);
}
