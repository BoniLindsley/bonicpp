#pragma once

// C++ standard libraries.
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <numeric>
#include <type_traits>
#include <vector>

namespace bonicpp {

template <typename T, std::size_t Rank> class Slice;

template <typename T, std::size_t Rank> class SliceIterator {
public:
  using self_type = SliceIterator;
  using pointer = T*;
  using reference =
      std::conditional_t<Rank == 1, T&, Slice<T, Rank - 1>>;
  using value_type =
      std::conditional_t<Rank == 1, T, Slice<T, Rank - 1>>;

public:
  SliceIterator() = default;
  SliceIterator(const self_type&) = default;
  SliceIterator(
      pointer ptr, const std::size_t* shape, std::size_t stride)
      : ptr_(ptr), shape_(shape), stride_(stride) {}

public:
  auto operator*() const -> reference {
    return deref(std::integral_constant<std::size_t, Rank>{});
  }
  auto operator++() -> SliceIterator& {
    ptr_ += stride_;
    return *this;
  }
  auto operator+(std::size_t n) const -> SliceIterator {
    return {ptr_ + n * stride_, shape_, stride_};
  }
  auto operator[](std::size_t n) const -> reference {
    return *(*this + n);
  }
  auto operator==(const SliceIterator& other) const -> bool {
    return ptr_ == other.ptr_;
  }
  auto operator!=(const SliceIterator& other) const -> bool {
    return not(*this == other);
  }

private:
  template <std::size_t R>
  auto deref(std::integral_constant<std::size_t, R>) const
      -> std::enable_if_t<(R > 1), Slice<T, Rank - 1>> {
    return Slice<T, Rank - 1>(ptr_, shape_ + 1);
  }
  template <std::size_t R>
  auto deref(std::integral_constant<std::size_t, R>) const
      -> std::enable_if_t<R == 1, T&> {
    return *ptr_;
  }

private:
  pointer ptr_ = nullptr;
  const std::size_t* shape_ = nullptr;
  std::size_t stride_ = 1;
};

template <typename T, std::size_t Rank> class Slice {
  static_assert(Rank >= 1, "Rank must be >= 1");

public:
  using self_type = Slice;
  using value_type = T;
  using pointer = T*;
  using iterator = SliceIterator<value_type, Rank>;
  using reference = typename iterator::reference;
  using const_iterator = SliceIterator<const value_type, Rank>;
  using const_reference = typename const_iterator::reference;

public:
  Slice(pointer ptr, const std::size_t* extents)
      : ptr_(ptr), extents_(extents) {
    stride_ = 1;
    for (auto i = 1u; i < Rank; ++i)
      stride_ *= extents_[i];
  }

public:
  auto operator[](std::size_t idx) -> reference { return at_index(idx); }
  auto operator[](std::size_t idx) const -> const_reference {
    return at_index(idx);
  }

public:
  auto begin() -> iterator { return {ptr_, extents_, stride()}; }
  auto begin() const -> const_iterator {
    return {ptr_, extents_, stride()};
  }
  auto cbegin() const -> const_iterator { return begin(); }
  auto cend() const -> const_iterator { return end(); }
  auto data() -> pointer { return ptr_; }
  auto data() const -> const pointer { return ptr_; }
  auto end() -> iterator { return {ptr_ + size(), extents_, stride()}; }
  auto end() const -> const_iterator {
    return {ptr_ + size(), extents_, stride()};
  }
  [[nodiscard]] auto extent(std::size_t axis) const -> std::size_t {
    return extents_[axis];
  }
  static constexpr auto rank() -> std::size_t { return Rank; }
  [[nodiscard]] auto size() const -> std::size_t {
    return extent(0) * stride();
  }
  [[nodiscard]] constexpr auto stride() const -> std::size_t {
    return stride_;
  }

private:
  template <std::size_t R = Rank>
  auto at_index(std::size_t idx)
      -> std::enable_if_t<(R > 1), reference> {
    assert(idx < extent(0));
    return {ptr_ + idx * stride(), extents_ + 1};
  }
  template <std::size_t R = Rank>
  auto at_index(std::size_t idx) const
      -> std::enable_if_t<(R > 1), const_reference> {
    assert(idx < extent(0));
    return {ptr_ + idx * stride(), extents_ + 1};
  }
  template <std::size_t R = Rank>
  auto at_index(std::size_t idx) -> std::enable_if_t<R == 1, reference> {
    assert(idx < extent(0));
    return ptr_[idx];
  }
  template <std::size_t R = Rank>
  auto at_index(std::size_t idx) const
      -> std::enable_if_t<R == 1, const_reference> {
    assert(idx < extent(0));
    return ptr_[idx];
  }

private:
  pointer ptr_;
  const std::size_t* extents_;
  std::size_t stride_;
};

template <typename T, std::size_t Rank> class MDVector {
  static_assert(Rank >= 1, "Rank must be >= 1");

public:
  using self_type = MDVector;
  using extents_type = std::array<std::size_t, Rank>;
  using value_type = T;
  using iterator = SliceIterator<value_type, Rank>;
  using reference = typename iterator::reference;
  using const_iterator = SliceIterator<const value_type, Rank>;
  using const_reference = typename const_iterator::reference;

public:
  MDVector() = default;
  explicit MDVector(extents_type extents)
      : extents_{extents},
        data_(
            std::accumulate(
                extents_.begin(), extents_.end(), std::size_t{1},
                std::multiplies<>{})) {}

public:
  auto operator[](std::size_t idx) -> reference {
    return to_slice()[idx];
  }
  auto operator[](std::size_t idx) const -> const_reference {
    return to_slice()[idx];
  }
  operator Slice<value_type, Rank>() { return to_slice(); }
  operator Slice<const value_type, Rank>() const { return to_slice(); }

public:
  auto begin() -> iterator { return to_slice().begin(); }
  auto begin() const -> const_iterator { return to_slice().begin(); }
  auto cbegin() const -> const_iterator { return begin(); }
  auto cend() const -> const_iterator { return to_slice().end(); }
  auto data() -> value_type* { return data_.data(); }
  auto data() const -> const value_type* { return data_.data(); }
  auto end() -> iterator { return to_slice().end(); }
  auto end() const -> const_iterator { return to_slice().end(); }
  [[nodiscard]] auto extent(std::size_t axis) const -> std::size_t {
    return extents_[axis];
  }
  auto fill(const value_type& value) -> self_type& {
    std::fill(data_.begin(), data_.end(), value);
    return *this;
  }
  static constexpr auto rank() -> std::size_t { return Rank; }
  auto extents() const -> const extents_type& { return extents_; }
  [[nodiscard]] auto size() const -> std::size_t { return data_.size(); }
  auto to_slice() -> Slice<value_type, Rank> {
    return {data_.data(), extents_.data()};
  }
  auto to_slice() const -> Slice<const value_type, Rank> {
    return {data_.data(), extents_.data()};
  }
  [[nodiscard]] auto stride() const -> std::size_t {
    if constexpr (Rank == 1) {
      return 1;
    }
    return std::accumulate(
        extents_.begin() + 1, extents_.end(), std::size_t{1},
        std::multiplies<>{});
  }

private:
  extents_type extents_{0};
  std::vector<value_type> data_{};
};

} // namespace bonicpp
