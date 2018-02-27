#pragma once
#ifndef MAL_READER_HPP
#define MAL_READER_HPP

#include <deque>
#include "hoolib.hpp"
#include "type.hpp"

class Reader {
private:
    std::deque<std::string> tokens_;

public:
    Reader(const std::string& src)
    {
        auto re = std::regex(
            R"([\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"|;.*|[^\s\[\]{}('"`,;)]*))");
        HooLib::search(HOOLIB_RANGE(src), re, [& tokens_ = tokens_](auto&& m) {
            auto str = m.str(1);
            if (!str.empty()) tokens_.emplace_back(str);
        });
    }

    const std::string& peek() const
    {
        HOOLIB_THROW_IF(tokens_.empty(), "no next expected token when peek")
        return tokens_.front();
    }

    std::string pop()
    {
        HOOLIB_THROW_IF(tokens_.empty(), "no next expected token when pop")
        // http://faithandbrave.hateblo.jp/entry/20130604/1370327651
        std::string ret(std::move(tokens_.front()));
        tokens_.pop_front();
        return ret;
    }

    std::shared_ptr<MalAtom> read_atom();
    std::shared_ptr<MalList> read_list();
    MalTypePtr read_form();
};

#endif