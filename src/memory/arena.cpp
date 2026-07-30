#include "arena.h"
#include <stdexcept>
#include <cstring>

Arena::Arena(size_t capacity)
    : buffer_(capacity), offset_(0)
{}

void* Arena::alloc(size_t size) {
    if (offset_ + size > buffer_.size()) {
        throw std::runtime_error("Arena overflow");
    }

    void* ptr = buffer_.data() + offset_;
    offset_ += size;
    return ptr;
}

void Arena::reset() {
    offset_ = 0;
}
