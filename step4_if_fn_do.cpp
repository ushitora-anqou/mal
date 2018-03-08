#include <iostream>
#include <string>
#include "env.hpp"
#include "factory.hpp"
#include "helper.hpp"
#include "reader.hpp"

MalTypePtr eval_special(MalTypePtr ast, EnvPtr env);

MalTypePtr READ(std::istream& is)
{
    std::string input;
    if (!std::getline(is, input)) return nullptr;
    auto reader = Reader(input);
    auto ast = reader.read_form();
    return ast;
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

int main()
{
    EnvPtr repl_env = mal::make_shared<Env>();
    auto ns = get_ns();
    for (auto && [ name, func ] : ns)
        repl_env->set(name, mal::make_shared<MalFunction>(func));
    // define not
    eval_str("(def! not (fn* (a) (if a false true)))", repl_env);

    while (true) {
        try {
            std::cout << "user> " << std::flush;
            if (rep(repl_env)) break;
        }
        catch (std::runtime_error& ex) {
            std::cerr << "RUNTIME_ERROR: " << ex.what() << std::endl;
        }
    }
    return 0;
}
