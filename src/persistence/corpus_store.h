#pragma once

#include <string>
#include <vector>

class CorpusStore {
public:
    // dir — директория, где лежит/будет лежать корпус
    explicit CorpusStore(const std::string& dir);

    // сохранить скрипт в корпус (и в память, и на диск)
    void save(const std::string& script);

    // получить случайный скрипт из корпуса (или пустую строку, если он пуст)
    std::string get_random() const;

    // размер корпуса (количество скриптов в памяти)
    size_t size() const;

private:
    std::string dir_;
    std::vector<std::string> corpus_;
};
