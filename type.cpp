#include "type.hpp"

MalTypePtr MalSymbol::eval(EnvPtr env) { return env->get(name_); }

MalTypePtr MalList::eval(EnvPtr env)
{
    std::vector<MalTypePtr> newList;
    for (auto&& item : list_) newList.push_back(item->eval(env));
    return std::make_shared<MalList>(newList);
}

MalTypePtr MalBuiltInFunction::call(EnvPtr env,
                                    const std::vector<MalTypePtr>& args)
{
    return func_(env, args);
}
