#include "tokenizer.h"
#include <cctype>

bool Tokenizer::is_identifier_start(char c) const {
    return std::isalpha(c) || c == '_' || c == '$';
}

bool Tokenizer::is_identifier_char(char c) const {
    return std::isalnum(c) || c == '_' || c == '$';
}

bool Tokenizer::is_digit(char c) const {
    return std::isdigit(c);
}

bool Tokenizer::is_operator(char c) const {
    static const std::string ops = "+-*/%=!<>&|^~?:";
    return ops.find(c) != std::string::npos;
}

bool Tokenizer::is_punctuation(char c) const {
    static const std::string p = "();{}[],.";
    return p.find(c) != std::string::npos;
}

bool Tokenizer::is_keyword(const std::string& s) const {
    static const std::vector<std::string> kw = {
        "let","var","const","function","return","if","else",
        "for","while","try","catch","throw","new","class",
        "import","export","async","await"
    };
    for (auto& k : kw)
        if (s == k) return true;
    return false;
}

std::vector<Token> Tokenizer::tokenize(const std::string& code) {
    std::vector<Token> out;
    size_t i = 0;

    while (i < code.size()) {
        char c = code[i];

        // пропуск пробелов
        if (std::isspace(c)) {
            i++;
            continue;
        }

        // строка
        if (c == '"' || c == '\'') {
            char quote = c;
            size_t start = i++;
            while (i < code.size() && code[i] != quote)
                i++;
            i++; // закрывающая кавычка

            out.push_back({TokenKind::String,
                           code.substr(start, i - start),
                           start});
            continue;
        }

        // число
        if (is_digit(c)) {
            size_t start = i;
            while (i < code.size() && is_digit(code[i]))
                i++;
            out.push_back({TokenKind::Number,
                           code.substr(start, i - start),
                           start});
            continue;
        }

        // идентификатор / ключевое слово
        if (is_identifier_start(c)) {
            size_t start = i;
            while (i < code.size() && is_identifier_char(code[i]))
                i++;

            std::string text = code.substr(start, i - start);
            TokenKind kind = is_keyword(text)
                ? TokenKind::Keyword
                : TokenKind::Identifier;

            out.push_back({kind, text, start});
            continue;
        }

        // оператор
        if (is_operator(c)) {
            out.push_back({TokenKind::Operator,
                           std::string(1, c),
                           i});
            i++;
            continue;
        }

        // пунктуация
        if (is_punctuation(c)) {
            out.push_back({TokenKind::Punctuation,
                           std::string(1, c),
                           i});
            i++;
            continue;
        }

        // неизвестное
        out.push_back({TokenKind::Unknown,
                       std::string(1, c),
                       i});
        i++;
    }

    return out;
}
