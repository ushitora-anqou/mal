#include "type.hpp"
#include "factory.hpp"
#include "helper.hpp"

MalTypePtr mal_eval_special(MalTypePtr ast, EnvPtr env);

MalTypePtr MalSymbol::eval(EnvPtr env) { return env->get(name_); }

std::string MalString::pr_str(bool print_readably) const
{
    if (mal::helper::is_keyword(data_))
        return mal::helper::keyword2string(data_);

    if (print_readably) return HooLib::cpp_escape_string(data_);
    return data_;
}

std::vector<MalTypePtr> MalSequential::eval_items(EnvPtr env)
{
    std::vector<MalTypePtr> newList;
    for (auto&& item : items_) newList.push_back(mal_eval(item, env));
    return newList;
}

bool MalSequential::is_equal_to(const MalTypePtr& rhs) const
{
    auto rhs_seq = rhs->as_sequential();
    if (!rhs_seq) return false;
    const auto& rhs_items = rhs_seq->items_;
    if (items_.size() != rhs_items.size()) return false;
    for (size_t i = 0; i < items_.size(); i++)
        if (!items_[i]->is_equal_to(rhs_items[i])) return false;
    return true;
}

MalTypePtr MalList::eval(EnvPtr env)
{
    return mal::make_shared<MalList>(eval_items(env));
}

MalTypePtr MalVector::eval(EnvPtr env)
{
    return mal::make_shared<MalVector>(eval_items(env));
}

///

MalTypePtr mal_eval(MalTypePtr ast, EnvPtr env)
{
    HOOLIB_THROW_UNLESS(ast, "invalid ast");

    if (auto ret = mal_eval_special(ast, env)) return ret;

    ast = ast->eval(env);  // eval_ast
    auto list_src = ast->as_list();
    if (!list_src || list_src->get().empty()) return ast;
    const auto& list = list_src->get();
    auto func = list[0]->as_function();
    HOOLIB_THROW_UNLESS(func, "invalid list: not function");

    return func->call(MalFunction::Args(list.begin() + 1, list.end()));
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

        auto let_env = mal::make_shared<Env>(env);
        for (auto it = bindings.begin(); it != bindings.end();) {
            auto key_symbol = (*it++)->as_symbol();
            HOOLIB_THROW_UNLESS(key_symbol, "invalid argument");
            auto value = mal_eval(*it++, let_env);
            let_env->set(key_symbol->name(), value);
        }

        return mal_eval(args[2], let_env);
    }

    if (name == "do") {
        MalTypePtr ret = mal::nil();
        for (auto it = args.begin(); ++it != args.end();)
            ret = mal_eval(*it, env);
        return ret;
    }

    if (name == "if") {
        HOOLIB_THROW_UNLESS(args.size() == 3 || args.size() == 4,
                            "invalid argument");
        auto cond = mal_eval(args[1], env);
        if (cond->as_nil() || cond->as_false()) {  // false
            if (args.size() == 3) return mal::nil();
            return mal_eval(args[3], env);
        }
        // true
        return mal_eval(args[2], env);
    }

    if (name == "fn*") {
        HOOLIB_THROW_UNLESS(args.size() == 3, "invalid number of arguments");
        auto seq = args[1]->as_sequential();
        HOOLIB_THROW_UNLESS(seq, "invalid argument");
        const auto& binds_src = seq->get();
        std::vector<std::string> binds;
        bool variadic = false;
        for (auto&& item : binds_src) {
            auto symbol = item->as_symbol();
            HOOLIB_THROW_UNLESS(symbol, "invalid argument");
            if (symbol->name() == "&") {
                variadic = true;
                break;
            }
            binds.push_back(symbol->name());
        }
        return mal::make_shared<MalFunction>([
            variadic, binds, outer_env = env, fn_body_ast = args[2]
        ](auto&& args) {
            // TODO:process for varadic
            auto env = mal::make_shared<Env>(outer_env, binds, args);
            return mal_eval(fn_body_ast, env);
        });
    }

    return nullptr;
}
