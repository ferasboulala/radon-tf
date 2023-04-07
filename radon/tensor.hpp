#pragma once

#include <array>
#include <cassert>
#include <numeric>
#include <vector>

#define ALWAYS_INLINE __attribute__((always_inline))

// This function is always inlined because we want to avoid having to recompute the accumulator
// values. Also, if the indices are the same, this whole thing will get optimized out from CSE.
template <unsigned RANK>
inline unsigned ravel(const std::array<unsigned, RANK>& indices,
                             const std::array<unsigned, RANK>& sizes) {
    static_assert(RANK);
    assert(indices.back() < sizes.back());
    unsigned index = indices.back();
    unsigned accumulator = sizes.back();
    for (int i = RANK - 2; i >= 0; --i) {
        assert(indices[i] < sizes[i]);
        index += accumulator * indices[i];
        accumulator *= sizes[i];
    }

    return index;
}

template <typename T, unsigned RANK>
class Tensor {
public:
    Tensor() = default;
    Tensor(const std::array<unsigned, RANK>& size, const T& filler = T()) { resize(size, filler); }
    ~Tensor() = default;

    ALWAYS_INLINE void resize(const std::array<unsigned, RANK>& size, const T& filler = T()) {
        size_ = size;
        const unsigned new_volume =
            std::accumulate(size.begin(), size.end(), 1, std::multiplies<unsigned>());
        data_.resize(new_volume, filler);
    }
    ALWAYS_INLINE const T& operator[](const std::array<unsigned, RANK>& indices) const {
        return data_[ravel<RANK>(indices, size_)];
    }
    ALWAYS_INLINE T& operator[](const std::array<unsigned, RANK>& indices) {
        return data_[ravel<RANK>(indices, size_)];
    }
    ALWAYS_INLINE unsigned volume() const { return data_.size(); }
    ALWAYS_INLINE const std::array<unsigned, RANK> size() const { return size_; }
    ALWAYS_INLINE const T*                         data() const { return data_.data(); }
    ALWAYS_INLINE T*                               data() { return data_.data(); }

private:
    std::array<unsigned, RANK> size_;
    std::vector<T>             data_;
};