#include "bounded_buffer.h"
#include <algorithm>

BoundedBuffer::BoundedBuffer(size_t capacity)
    : buf_(capacity)
    , capacity_(capacity)
    , size_(0)
{}

void BoundedBuffer::append(const char* data, size_t len) {
    if (len == 0 || capacity_ == 0) return;

    // если данных больше capacity — берём только последние
    if (len > capacity_) {
        data += (len - capacity_);
        len = capacity_;
    }

    // если переполнение — сдвигаем существующие данные
    if (size_ + len > capacity_) {
        size_t overflow = (size_ + len) - capacity_;
        std::move(buf_.begin() + overflow, buf_.begin() + size_, buf_.begin());
        size_ -= overflow;
    }

    // копируем новые данные в конец
    std::copy(data, data + len, buf_.begin() + size_);
    size_ += len;
}

void BoundedBuffer::append(const std::string& s) {
    append(s.data(), s.size());
}

std::string BoundedBuffer::str() const {
    return std::string(buf_.begin(), buf_.begin() + size_);
}
