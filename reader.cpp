#include "reader.hpp"

std::shared_ptr<MalAtom> Reader::read_atom()
{
    auto token = pop();
    if ('0' <= token[0] && token[0] <= '9')  // number
        return std::make_shared<MalInteger>(std::stoi(token));
    else
        return std::make_shared<MalSymbol>(token);
}

std::shared_ptr<MalList> Reader::read_list()
{
    pop();  // pop "("
    std::vector<MalTypePtr> list;
    while (peek() != ")") list.push_back(read_form());
    pop();
    return std::make_shared<MalList>(std::move(list));
}

MalTypePtr Reader::read_form()
{
    if (peek() == "(") return read_list();
    return read_atom();
}
