#include "reader.hpp"
#include <fstream>
#include "factory.hpp"
#include "helper.hpp"

MalTypePtr Reader::parse()
{
    if (tokens_.empty()) return mal::nil();
    return read_form();
}

std::shared_ptr<MalType> Reader::read_atom()
{
    auto token = pop();
    if ('0' <= token[0] && token[0] <= '9')  // number
        return mal::make_shared<MalInteger>(HooLib::str2int(token));
    else if (token[0] == '"')  // string
        return mal::make_shared<MalString>(HooLib::cpp_unescape_string(token));
    else if (token[0] == ':')  // keyword
        return mal::make_shared<MalString>(mal::helper::string2keyword(token));
    else if (token == "nil")
        return mal::make_shared<MalNil>();
    else if (token == "true")
        return mal::make_shared<MalTrue>();
    else if (token == "false")
        return mal::make_shared<MalFalse>();
    else
        return mal::make_shared<MalSymbol>(token);
}

std::vector<MalTypePtr> Reader::read_list_items(const std::string& end_token)
{
    pop();
    std::vector<MalTypePtr> items;
    while (peek() != end_token) {
        auto ast = read_form();
        HOOLIB_THROW_UNLESS(ast, "invalid ast");
        items.push_back(ast);
    }
    pop();
    return items;
}

MalTypePtr Reader::read_form()
{
    auto next = peek();
    if (next == "(") return mal::make_shared<MalList>(read_list_items(")"));
    if (next == "[") return mal::make_shared<MalVector>(read_list_items("]"));
    if (next == "@") {
        pop();
        auto ast = read_form();
        return mal::list({mal::symbol("deref"), ast});
    }
    if (next == "'") {
        pop();
        auto ast = read_form();
        return mal::list({mal::symbol("quote"), ast});
    }
    if (next == "`") {
        pop();
        auto ast = read_form();
        return mal::list({mal::symbol("quasiquote"), ast});
    }
    if (next == "~") {
        pop();
        auto ast = read_form();
        return mal::list({mal::symbol("unquote"), ast});
    }
    if (next == "~@") {
        pop();
        auto ast = read_form();
        return mal::list({mal::symbol("splice-unquote"), ast});
    }

    return read_atom();
}
