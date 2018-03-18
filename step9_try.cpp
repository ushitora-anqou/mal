#include <iostream>
#include <string>
#include "env.hpp"
#include "exception.hpp"
#include "factory.hpp"
#include "helper.hpp"
#include "reader.hpp"

MalTypePtr eval_special(MalTypePtr ast, EnvPtr env);

MalTypePtr READ(std::istream& is)
{
    std::string input;
    if (!std::getline(is, input)) return nullptr;
    return Reader(input).parse();
}

MalTypePtr EVAL(MalTypePtr ast, EnvPtr env) { return mal_eval(ast, env); }

void PRINT(MalTypePtr ast, std::ostream& os)
{
    HOOLIB_THROW_UNLESS(ast, "invalid ast");
    os << ast->pr_str(true) << std::endl;
}

std::unordered_map<std::string, MalFunction::Func> get_ns()
{
    return {
        {"+",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2, "invalid argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::int_(lhs->get() + rhs->get());
         }},
        {"-",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2, "invalid argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::int_(lhs->get() - rhs->get());
         }},
        {"*",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2, "invalid argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::int_(lhs->get() * rhs->get());
         }},
        {"/",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2, "invalid argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::int_(lhs->get() / rhs->get());
         }},

        {"pr-str",
         [](auto&& args) {
             return mal::make_shared<MalString>(
                 HooLib::join(HOOLIB_RANGE(args), " ",
                              [](auto&& item) { return item->pr_str(true); }));
         }},
        {"str",
         [](auto&& args) {
             return mal::make_shared<MalString>(
                 HooLib::join(HOOLIB_RANGE(args), "",
                              [](auto&& item) { return item->pr_str(false); }));
         }},
        {"prn",
         [](auto&& args) {
             std::cout << HooLib::join(
                              HOOLIB_RANGE(args), " ",
                              [](auto&& item) { return item->pr_str(true); })
                       << std::endl;
             return mal::nil();
         }},
        {"println",
         [](auto&& args) {
             std::cout << HooLib::join(
                              HOOLIB_RANGE(args), " ",
                              [](auto&& item) { return item->pr_str(false); })
                       << std::endl;
             return mal::nil();
         }},

        {"list",
         [](auto&& args) {
             auto src = std::vector<MalTypePtr>(args.begin(), args.end());
             return mal::make_shared<MalList>(src);
         }},
        {"list?",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             return mal::boolean(bool(args[0]->as_list()));
         }},

        {"empty?",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             auto seq = args[0]->as_sequential();
             HOOLIB_THROW_UNLESS(seq, "invalid argument");
             return mal::boolean(seq->get().empty());
         }},

        {"count",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             if (args[0]->as_nil()) return mal::int_(0);
             auto seq = args[0]->as_sequential();
             HOOLIB_THROW_UNLESS(seq, "invalid argument");
             return mal::int_(seq->get().size());
         }},

        {"cons",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of arguments");
             auto src_list = args[1]->as_sequential();
             HOOLIB_THROW_UNLESS(src_list, "invalid argument");
             std::vector<MalTypePtr> new_list;
             new_list.push_back(args[0]);
             std::copy(HOOLIB_RANGE(src_list->get()),
                       std::back_inserter(new_list));
             return mal::list(std::move(new_list));
         }},

        {"concat",
         [](auto&& args) {
             std::vector<MalTypePtr> ret_list;
             for (auto&& arg : args) {
                 auto src_list = arg->as_sequential();
                 HOOLIB_THROW_UNLESS(src_list, "invalid argument");
                 std::copy(HOOLIB_RANGE(src_list->get()),
                           std::back_inserter(ret_list));
             }
             return mal::list(std::move(ret_list));
         }},

        {"read-string",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             auto src = args[0]->as_string();
             HOOLIB_THROW_UNLESS(src, "invalid argument");
             return Reader(src->get()).parse();
         }},
        {"slurp",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             return mal::string(
                 HooLib::read_file_all(args[0]->as_string()->get()));
         }},

        {"nth",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of argument");
             auto seq = args[0]->as_sequential();
             auto idx = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(
                 seq && idx &&
                     static_cast<size_t>(idx->get()) < seq->get().size(),
                 "invalid argument");
             return seq->get()[idx->get()];
         }},
        {"first",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             auto seq = args[0]->as_sequential();
             auto nil = args[0]->as_nil();
             HOOLIB_THROW_UNLESS(seq || nil, "invalid argument");
             if (nil || seq->get().empty())
                 return std::static_pointer_cast<MalType>(mal::nil());
             return seq->get()[0];
         }},
        {"rest",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             auto seq = args[0]->as_sequential();
             if (!seq) return mal::list();
             if (seq->get().size() <= 1) return mal::list();
             return mal::list(std::vector<MalTypePtr>(seq->get().begin() + 1,
                                                      seq->get().end()));
         }},

        {"=",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of argument");
             return mal::boolean(args[0]->is_equal_to(args[1]));
         }},
        {"<",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::boolean(lhs->get() < rhs->get());
         }},
        {"<=",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::boolean(lhs->get() <= rhs->get());
         }},
        {">",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::boolean(lhs->get() > rhs->get());
         }},
        {">=",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of argument");
             auto lhs = args[0]->as_integer();
             auto rhs = args[1]->as_integer();
             HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
             return mal::boolean(lhs->get() >= rhs->get());
         }},

        {"atom",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             HOOLIB_THROW_UNLESS(args[0], "invalid argument");
             return mal::atom(args[0]);
         }},
        {"atom?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             auto value = args[0]->as_atom();
             return mal::boolean(value != nullptr);
         }},
        {"deref",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of argument");
             auto src = args[0]->as_atom();
             HOOLIB_THROW_UNLESS(src, "invalid argument");
             return src->deref();
         }},
        {"reset!",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of argument");
             auto atm = args[0]->as_atom();
             auto val = args[1];
             HOOLIB_THROW_UNLESS(atm && val, "invalid argument");
             atm->set_ref(val);
             return val;
         }},
        {"swap!",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() >= 2,
                                 "invalid number of arguments");
             auto atm = args[0]->as_atom();
             auto func = args[1]->as_function();
             HOOLIB_THROW_UNLESS(atm && func, "invalid argument");

             // create the argument
             std::vector<MalTypePtr> new_args;
             new_args.push_back(atm->deref());
             std::copy(args.begin() + 2, args.end(),
                       std::back_inserter(new_args));

             auto res = func->call(MalFunction::Args(HOOLIB_RANGE(new_args)));
             atm->set_ref(res);
             return res;
         }},

        {"throw",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             MAL_THROW(args[0]);
             return mal::nil();  // dummy
         }},

        {"apply",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() >= 2,
                                 "invalid number of arguments");
             auto func = args[0]->as_function();
             HOOLIB_THROW_UNLESS(func, "invalid argument");
             auto seq = (*(args.end() - 1))->as_sequential();
             HOOLIB_THROW_UNLESS(seq, "invalid argument");
             std::vector<MalTypePtr> list;
             for (size_t i = 1; i < args.size() - 1; i++)
                 list.push_back(args[i]);
             std::copy(HOOLIB_RANGE(seq->get()), std::back_inserter(list));
             return func->call(MalFunction::Args(HOOLIB_RANGE(list)));
         }},
        {"map",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of arguments");
             auto func = args[0]->as_function();
             auto list = args[1]->as_sequential();
             std::vector<MalTypePtr> ret_src;
             std::transform(
                 HOOLIB_RANGE(list->get()), std::back_inserter(ret_src),
                 [&func](auto&& item) {
                     std::vector<MalTypePtr> args;
                     args.push_back(item);
                     return func->call(MalFunction::Args(HOOLIB_RANGE(args)));
                 });
             return mal::list(ret_src);
         }},

        {"nil?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             return mal::boolean(args[0]->as_nil() != nullptr);
         }},
        {"true?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             return mal::boolean(args[0]->as_true() != nullptr);
         }},
        {"false?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             return mal::boolean(args[0]->as_false() != nullptr);
         }},
        {"symbol?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             return mal::boolean(args[0]->as_symbol() != nullptr);
         }},

        {"symbol",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             auto name = args[0]->as_string();
             HOOLIB_THROW_UNLESS(name, "invalid argument");
             return mal::symbol(name->get());
         }},
        {"keyword",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             auto name = args[0]->as_string();
             HOOLIB_THROW_UNLESS(name, "invalid argument");
             if (mal::helper::is_keyword(name->get())) return name;
             return mal::keyword(mal::helper::string2keyword(name->get()));
         }},
        {"keyword?",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             auto name = args[0]->as_string();
             if (name == nullptr) return mal::false_();
             return mal::boolean(mal::helper::is_keyword(name->get()));
         }},
        {"vector",
         [](auto&& args) {
             return mal::vector(std::vector<MalTypePtr>(HOOLIB_RANGE(args)));
         }},
        {"vector?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             return mal::boolean(args[0]->as_vector() != nullptr);
         }},
        {"sequential?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             return mal::boolean(args[0]->as_sequential() != nullptr);
         }},

        {"hash-map",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() % 2 == 0,
                                 "invalid number of arguments");
             return mal::hash_map(
                 mal::helper::make_hash_map_container(HOOLIB_RANGE(args)));
         }},
        {"map?",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             return mal::boolean(args[0]->as_hash_map() != nullptr);
         }},
        {"assoc",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() >= 1 && args.size() % 2 == 1,
                                 "invalid number of arguments");
             auto org_hash = args[0]->as_hash_map();
             HOOLIB_THROW_UNLESS(org_hash, "invalid argument");
             auto src = org_hash->data();
             mal::helper::insert_odd_even_list(src, args.begin() + 1,
                                               args.end());
             return mal::hash_map(src);
         }},
        {"dissoc",
         [](auto&& args) {
             HOOLIB_THROW_UNLESS(args.size() >= 1,
                                 "invalid number of arguments");
             auto org_hash = args[0]->as_hash_map();
             HOOLIB_THROW_UNLESS(org_hash, "invalid argument");
             auto src = org_hash->data();
             for (auto it = args.begin() + 1; it != args.end(); ++it) {
                 auto key = (*it)->as_string();
                 HOOLIB_THROW_UNLESS(key, "invalid argument");
                 src.erase(key->get());
             }
             return mal::hash_map(src);
         }},
        {"get",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of arguments");
             if (args[0]->as_nil()) return mal::nil();
             auto hash = args[0]->as_hash_map();
             auto key = args[1]->as_string();
             HOOLIB_THROW_UNLESS(hash && key, "invalid argument");
             auto ret = hash->get_if(key->get());
             if (ret == nullptr) return mal::nil();
             return ret;
         }},
        {"contains?",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 2,
                                 "invalid number of arguments");
             auto hash = args[0]->as_hash_map();
             auto key = args[1]->as_string();
             HOOLIB_THROW_UNLESS(hash && key, "invalid argument");
             auto ret = hash->get_if(key->get());
             return mal::boolean(ret != nullptr);
         }},
        {"keys",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             auto hash = args[0]->as_hash_map();
             HOOLIB_THROW_UNLESS(hash, "invalid argument");
             std::vector<MalTypePtr> src;
             for (auto && [ k, v ] : hash->data())
                 src.push_back(mal::string(k));
             return mal::list(src);
         }},
        {"vals",
         [](auto&& args) -> MalTypePtr {
             HOOLIB_THROW_UNLESS(args.size() == 1,
                                 "invalid number of arguments");
             auto hash = args[0]->as_hash_map();
             HOOLIB_THROW_UNLESS(hash, "invalid argument");
             std::vector<MalTypePtr> src;
             for (auto && [ k, v ] : hash->data()) src.push_back(v);
             return mal::list(src);
         }},

    };
}

MalTypePtr eval_str(const std::string& src, const EnvPtr& env)
{
    std::stringstream ss(src);
    auto ast = READ(ss);
    HOOLIB_THROW_UNLESS(ast, "invalid src");
    return EVAL(ast, env);
}

int rep(const EnvPtr& repl_env, std::istream& is = std::cin)
{
    auto ast = READ(is);
    if (!ast) return 1;
    PRINT(EVAL(ast, repl_env), std::cout);
    return 0;
}

int main(int argc, char** argv)
{
    EnvPtr repl_env = mal::make_shared<Env>();
    auto ns = get_ns();
    for (auto && [ name, func ] : ns)
        repl_env->set(name, mal::make_shared<MalFunction>(func));

    // define eval
    repl_env->set("eval",
                  mal::make_shared<MalFunction>([&repl_env](auto&& args) {
                      HOOLIB_THROW_UNLESS(args.size() == 1,
                                          "invalid number of arguments");
                      return mal_eval(args[0], repl_env);
                  }));

    // define not
    eval_str("(def! not (fn* (a) (if a false true)))", repl_env);

    // define load-file
    eval_str(
        R"***((def! load-file (fn* (f) (eval (read-string (str "(do " (slurp f) ")"))))))***",
        repl_env);

    // define *ARGV*
    std::vector<MalTypePtr> argv_list;
    for (int i = 2; i < argc; i++) argv_list.push_back(mal::string(argv[i]));
    repl_env->set("*ARGV*", mal::list(argv_list));

    // define cond
    eval_str(
        R"***((defmacro! cond (fn* (& xs) (if (> (count xs) 0) (list 'if (first xs) (if (> (count xs) 1) (nth xs 1) (throw "odd number of forms to cond")) (cons 'cond (rest (rest xs)))))))))***",
        repl_env);

    // define or
    eval_str(
        R"***((defmacro! or (fn* (& xs) (if (empty? xs) nil (if (= 1 (count xs)) (first xs) `(let* (or_FIXME ~(first xs)) (if or_FIXME or_FIXME (or ~@(rest xs))))))))))***",
        repl_env);

    if (argc == 1) {  // REPL
        while (true) {
            try {
                std::cout << "user> " << std::flush;
                if (rep(repl_env)) break;
            }
            catch (mal::Exception& ex) {
                std::cerr << "RUNTIME_ERROR: " << ex.get()->pr_str(true)
                          << std::endl;
            }
            catch (std::runtime_error& ex) {
                std::cerr << "RUNTIME_ERROR: " << ex.what() << std::endl;
            }
        }
    }
    else {
        try {
            std::stringstream ss;
            ss << "(load-file \"" << HooLib::cpp_escape_string(argv[0]) << "\")"
               << std::endl;
            eval_str(ss.str(), repl_env);
        }
        catch (std::runtime_error& ex) {
            std::cerr << "RUNTIME_ERROR: " << ex.what() << std::endl;
        }
    }

    return 0;
}
