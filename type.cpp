#include "type.hpp"

MalTypePtr mal_eval_special(MalTypePtr ast, EnvPtr env);

MalTypePtr MalSymbol::eval(EnvPtr env) { return env->get(name_); }

MalTypePtr MalList::eval(EnvPtr env)
{
    std::vector<MalTypePtr> newList;
    for (auto&& item : list_) newList.push_back(mal_eval(item, env));
    return std::make_shared<MalList>(newList);
}

MalTypePtr MalBuiltInFunction::call(EnvPtr env,
                                    const std::vector<MalTypePtr>& args)
{
    return func_(env, args);
}

MalTypePtr mal_eval(MalTypePtr ast, EnvPtr env)
{
    HOOLIB_THROW_UNLESS(ast, "invalid ast");

    // def! and let*
    if (auto ret = mal_eval_special(ast, env)) return ret;

    ast = ast->eval(env);  // eval_ast
    auto list = ast->as_list();
    if (!list || list->get().empty()) return ast;
    auto func = list->get()[0]->as_function();
    HOOLIB_THROW_UNLESS(func, "invalid list: not function");

    return func->call(env, list->get());
}

MalTypePtr mal_eval_special(MalTypePtr ast, EnvPtr env)
{
    auto list = ast->as_list();
    if (!list || list->get().empty()) return nullptr;
    auto args = list->get();
    auto symbol = args[0]->as_symbol();
    if (!symbol) return nullptr;
    auto name = symbol->name();

    if (name == "def!") {
        HOOLIB_THROW_UNLESS(args.size() == 3, "invalid number of argument");
        auto key_symbol = args[1]->as_symbol();
        HOOLIB_THROW_UNLESS(key_symbol, "invalid argument");
        auto value = mal_eval(args[2], env);
        HOOLIB_THROW_UNLESS(value, "invalid argument");
        env->set(key_symbol->name(), value);
        return value;
    }

    if (name == "let*") {
        HOOLIB_THROW_UNLESS(args.size() == 3, "invalid number of argument");
        auto bindings_src = args[1]->as_list();
        HOOLIB_THROW_UNLESS(bindings_src, "invalid argument");
        auto bindings = bindings_src->get();
        HOOLIB_THROW_UNLESS(bindings.size() % 2 == 0, "invalid argument");

        auto let_env = std::make_shared<Env>(env);
        for (int i = 0; i < bindings.size() / 2; i++) {
            auto key_symbol = bindings[i * 2]->as_symbol();
            HOOLIB_THROW_UNLESS(key_symbol, "invalid argument");
            auto value = mal_eval(bindings[i * 2 + 1], let_env);
            let_env->set(key_symbol->name(), value);
        }

        return mal_eval(args[2], let_env);
    }

    return nullptr;
}
