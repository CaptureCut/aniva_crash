#pragma once
#include <string>
#include <vector>

class BoundedBuffer {
public:
    explicit BoundedBuffer(size_t capacity);

    void append(const char* data, size_t len);
    void append(const std::string& s);

    std::string str() const;

private:
    std::vector<char> buf_;
    size_t capacity_;
    size_t size_;
};
