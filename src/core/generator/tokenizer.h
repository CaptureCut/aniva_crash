#pragma once
#include <string>
#include <vector>

enum class TokenKind {
    Identifier,
    Number,
    String,
    Operator,
    Punctuation,
    Keyword,
    Unknown
};

struct Token {
    TokenKind kind;
    std::string text;
    size_t position;
};

class Tokenizer {
public:
    Tokenizer() = default;

    // Разбивает JS-код на токены
    std::vector<Token> tokenize(const std::string& code);

private:
    bool is_identifier_start(char c) const;
    bool is_identifier_char(char c) const;
    bool is_digit(char c) const;
    bool is_operator(char c) const;
    bool is_punctuation(char c) const;
    bool is_keyword(const std::string& s) const;
};
