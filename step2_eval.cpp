#include <iostream>
#include <string>
#include "env.hpp"
#include "reader.hpp"

MalTypePtr eval_ast(EnvPtr env, MalTypePtr ast);

MalTypePtr READ()
{
    std::cout << "user> " << std::flush;
    std::string input;
    if (!std::getline(std::cin, input)) return nullptr;
    auto reader = Reader(input);
    auto ast = reader.read_form();
    return ast;
}

MalTypePtr EVAL(MalTypePtr ast)
{
    EnvPtr globalEnv = std::make_shared<Env>();
    globalEnv->set(
        "+",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() + rhs->get());
            }));
    globalEnv->set(
        "-",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() - rhs->get());
            }));
    globalEnv->set(
        "*",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() * rhs->get());
            }));
    globalEnv->set(
        "/",
        std::make_shared<MalBuiltInFunction>(
            [](EnvPtr env, const std::vector<MalTypePtr>& args) {
                HOOLIB_THROW_UNLESS(args.size() == 2 + 1, "invalid argument");
                auto lhs = args[1]->as_integer();
                auto rhs = args[2]->as_integer();
                HOOLIB_THROW_UNLESS(lhs && rhs, "invalid argument");
                return std::make_shared<MalInteger>(lhs->get() / rhs->get());
            }));

    // def! and let*

    ast = ast->eval(globalEnv);  // eval_ast
    auto list = ast->as_list();
    if (!list || list->get().empty()) return list;
    auto func = list->get()[0]->as_function();
    HOOLIB_THROW_UNLESS(func, "invalid list: not function");

    return func->call(globalEnv, list->get());
}

void PRINT(MalTypePtr ast) { std::cout << ast->pr_str() << std::endl; }

int main()
{
    while (true) {
        try {
            auto ast = READ();
            if (!ast) break;
            PRINT(EVAL(ast));
        }
        catch (std::runtime_error& ex) {
            std::cerr << "RUNTIME_ERROR: " << ex.what() << std::endl;
        }
    }
    return 0;
}
