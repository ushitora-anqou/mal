#include <iostream>
#include <string>
#include "env.hpp"
#include "reader.hpp"

MalTypePtr eval_special(MalTypePtr ast, EnvPtr env);

MalTypePtr READ()
{
    std::cout << "user> " << std::flush;
    std::string input;
    if (!std::getline(std::cin, input)) return nullptr;
    auto reader = Reader(input);
    auto ast = reader.read_form();
    return ast;
}

MalTypePtr EVAL(MalTypePtr ast, EnvPtr env) { return mal_eval(ast, env); }

void PRINT(MalTypePtr ast)
{
    if (ast) std::cout << ast->pr_str() << std::endl;
}

int main()
{
    EnvPtr global_env = std::make_shared<Env>();
    global_env->set(
        "+",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() + rhs->get());
            }));
    global_env->set(
        "-",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() - rhs->get());
            }));
    global_env->set(
        "*",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() * rhs->get());
            }));
    global_env->set(
        "/",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() / rhs->get());
            }));

    while (true) {
        try {
            auto ast = READ();
            if (!ast) break;
            PRINT(EVAL(ast, global_env));
        }
        catch (std::runtime_error& ex) {
            std::cerr << "RUNTIME_ERROR: " << ex.what() << std::endl;
        }
    }
    return 0;
}
