#include "type.hpp"
#include "factory.hpp"
#include "helper.hpp"

using TCOSwitch = std::variant<MalTypePtr, std::tuple<MalTypePtr, EnvPtr>>;

TCOSwitch mal_eval_special(MalTypePtr ast, EnvPtr env);
MalTypePtr quasiquote(const MalTypePtr& ast);
MalTypePtr macroexpand(MalTypePtr ast, const EnvPtr& env);

MalTypePtr MalSymbol::eval(EnvPtr env) { return env->get(name_); }

std::string MalAtom::pr_str(bool print_readably) const
{
    std::stringstream ss;
    ss << "(atom " << ref_->pr_str(print_readably) << ")";
    return ss.str();
}

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
    while (true) {
        HOOLIB_THROW_UNLESS(ast, "invalid ast");

        // macro expansion
        ast = macroexpand(ast, env);

        {
            // special eval
            auto ret = mal_eval_special(ast, env);
            switch (ret.index()) {
                case 0:
                    if (auto value = std::get<0>(ret)) return value;
                    break;

                case 1:
                    std::tie(ast, env) = std::get<1>(ret);
                    continue;
            }
        }

        auto ast_list = ast->as_list();
        if (!ast_list) return ast->eval(env);
        if (ast_list->get().empty()) return ast;

        ast_list = ast_list->eval(env)->as_list();
        const auto& list = ast_list->get();
        auto func = list[0]->as_function();
        HOOLIB_THROW_UNLESS(func, "invalid list: not function");

        return func->call(MalFunction::Args(list.begin() + 1, list.end()));
    }
}

TCOSwitch mal_eval_special(MalTypePtr ast, EnvPtr env)
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

    if (name == "defmacro!") {
        HOOLIB_THROW_UNLESS(args.size() == 3, "invalid number of argument");
        auto key_symbol = args[1]->as_symbol();
        HOOLIB_THROW_UNLESS(key_symbol, "invalid argument");
        auto func = mal_eval(args[2], env)->as_function();
        HOOLIB_THROW_UNLESS(func, "invalid argument");
        func->set_macro();
        env->set(key_symbol->name(), func);
        return func;
    }

    if (name == "let*") {
        HOOLIB_THROW_UNLESS(args.size() == 3, "invalid number of argument");
        auto bindings_src = args[1]->as_sequential();
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

        return std::make_tuple(args[2], let_env);
    }

    if (name == "do") {
        auto size = args.size();
        if (size == 1) return mal::nil();
        if (size == 2) return std::make_tuple(args[1], env);

        auto it = args.begin();
        auto end = args.end();
        --end;
        auto ret = mal::nil();
        for (; ++it != end;) mal_eval(*it, env);
        return std::make_tuple(*end, env);
    }

    if (name == "if") {
        HOOLIB_THROW_UNLESS(args.size() == 3 || args.size() == 4,
                            "invalid argument");
        auto cond = mal_eval(args[1], env);
        if (cond->as_nil() || cond->as_false()) {  // false
            if (args.size() == 3) return mal::nil();
            return std::make_tuple(args[3], env);
        }
        // true
        return std::make_tuple(args[2], env);
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
        if (variadic) {
            HOOLIB_THROW_UNLESS(binds.size() + 2 == binds_src.size(),
                                "invalid argument");
            auto symbol = binds_src.back()->as_symbol();
            HOOLIB_THROW_UNLESS(symbol, "invalid argument");
            binds.push_back(symbol->name());
        }

        return mal::make_shared<MalFunction>([
            variadic, binds, outer_env = env, fn_body_ast = args[2]
        ](auto&& args) {
            HOOLIB_THROW_UNLESS((variadic && args.size() >= binds.size() - 1) ||
                                    (!variadic && args.size() == binds.size()),
                                "invalid argument");
            auto env = mal::make_shared<Env>(outer_env);
            for (size_t i = 0; i < (variadic ? binds.size() - 1 : binds.size());
                 i++)
                env->set(binds[i], args[i]);
            if (variadic)
                env->set(binds.back(),
                         mal::list(std::vector<MalTypePtr>(
                             args.begin() + binds.size() - 1, args.end())));
            return mal_eval(fn_body_ast, env);
        });
    }

    if (name == "quote") {
        HOOLIB_THROW_UNLESS(args.size() == 2, "invalid number of arguments");
        return args[1];
    }

    if (name == "quasiquote") {
        HOOLIB_THROW_UNLESS(args.size() == 2, "invalid number of arguments");
        return std::make_tuple(quasiquote(args[1]), env);
    }

    if (name == "macroexpand") {
        HOOLIB_THROW_UNLESS(args.size() == 2, "invalid number of arguments");
        return macroexpand(args[1], env);
    }

    return nullptr;
}

MalTypePtr quasiquote(const MalTypePtr& ast)
{
    if (!mal::helper::is_pair(ast))
        return mal::list({mal::symbol("quote"), ast});

    auto ast_seq = ast->as_sequential()->get();
    if (auto symbol = ast_seq[0]->as_symbol()) {
        if (symbol->name() == "unquote") {
            HOOLIB_THROW_UNLESS(ast_seq.size() == 2, "invalid argument");
            return ast_seq[1];
        }
    }

    if (mal::helper::is_pair(ast_seq[0])) {
        auto ast_seq_0_seq = ast_seq[0]->as_sequential()->get();
        if (auto symbol = ast_seq_0_seq[0]->as_symbol()) {
            if (symbol->name() == "splice-unquote") {
                std::vector<MalTypePtr> list(ast_seq.begin() + 1,
                                             ast_seq.end());
                return mal::list({mal::symbol("concat"), ast_seq_0_seq[1],
                                  quasiquote(mal::list(list))});
            }
        }
    }

    std::vector<MalTypePtr> list(ast_seq.begin() + 1, ast_seq.end());
    return mal::list({mal::symbol("cons"), quasiquote(ast_seq[0]),
                      quasiquote(mal::list(list))});
}

bool is_macro_call(const MalTypePtr& ast, const EnvPtr& env)
{
    auto list = ast->as_list();
    if (!list || list->get().empty()) return false;
    auto symbol = list->get()[0]->as_symbol();
    if (!symbol) return false;
    auto func_src = env->get_if(symbol->name());
    if (!func_src) return false;
    auto func = func_src->as_function();
    if (!func) return false;
    return func->is_macro();
}

MalTypePtr macroexpand(MalTypePtr ast, const EnvPtr& env)
{
    while (is_macro_call(ast, env)) {
        auto list = ast->as_list()->get();
        auto func = env->get(list[0]->as_symbol()->name())->as_function();
        ast = func->call(MalFunction::Args(list.begin() + 1, list.end()));
    }

    return ast;
}
