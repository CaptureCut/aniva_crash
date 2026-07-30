#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

class Arena {
public:
    explicit Arena(size_t capacity = 1024 * 1024); // 1 MB default

    void* alloc(size_t size);
    void reset();

    size_t used() const { return offset_; }
    size_t capacity() const { return buffer_.size(); }

private:
    std::vector<uint8_t> buffer_;
    size_t offset_;
};
