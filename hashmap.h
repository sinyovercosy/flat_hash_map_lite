#include <cassert>
#include <concepts>
#include <memory>
#include <vector>

namespace ty {

template <typename K, typename V, typename Hash = std::hash<K>, typename Eq = std::equal_to<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  requires std::equality_comparable<K> && std::is_nothrow_invocable_r_v<std::size_t, Hash, K> && std::is_move_constructible_v<std::pair<const K, V>>
class flat_hash_map_lite {
 public:
  using value_type = std::pair<const K, V>;

  bool empty() const { return size_ == 0; }

  // TODO: 1) return an iterator, and 2) add const version
  value_type* find(K const& key) {
    if (empty()) {
      return nullptr;  // special case where capacity_ == 1 but we have not allocated anything
    }
    std::size_t i = FindIndex(key);
    return ctrl_[i] ? std::addressof(data_[i]) : nullptr;
  }

  /**
   * inserts a new element iff the key does not already exist in the map
   * @returns: (pointer to the inserted element or an existing element with equal key, a boolean indicating if insertion took place)
   */
  std::pair<value_type*, bool> insert(value_type kv) {
    if (size_ + 1 >= capacity_) {
      Rehash();  // assume that we are likely inserting a new element
    }
    std::size_t i = FindIndex(kv.first);
    if (ctrl_[i]) {
      return {std::addressof(data_[i]), false};
    }
    size_++;
    ctrl_[i] = true;
    std::construct_at(std::addressof(data_[i]), std::move(kv));
    return {std::addressof(data_[i]), true};
  }

 private:
  std::size_t Clamp(std::size_t i, std::size_t capacity) const { return i & (capacity - 1); }
  std::size_t ResizeThreshold() const {
    // max_load_factor = 0.875
    return capacity_ - (capacity_ >> 3);
  }

  struct ProbeSeq {
    std::size_t NextOffset() {
      i++;
      std::size_t ret = i * i;
      return ret;
    }
    std::uint32_t i = 0;
  };

  std::size_t FindIndex(K const& key) {
    assert(capacity_ > 0);
    const std::size_t origin = Clamp(Hash{}(key), capacity_);
    ProbeSeq seq;
    std::size_t i = origin;
    while (ctrl_[i]) {
      if (data_[i].first == key) {
        return i;
      } else {
        i = Clamp(origin + seq.NextOffset(), capacity_);
      }
    }
    return i;
  }

  void Rehash() {
    const std::size_t new_capacity = capacity_ * 2;
    std::vector<bool, Allocator> new_ctrl(new_capacity, allocator_);
    value_type* new_data = allocator_.allocate(new_capacity);
    for (std::size_t i = 0; i < ctrl_.size(); ++i) {
      if (!ctrl_[i]) {
        continue;
      }
      const std::size_t origin = Clamp(Hash{}(data_[i].first), new_capacity);
      ProbeSeq seq;
      std::size_t new_i = origin;
      while (new_ctrl[new_i]) {
        new_i = Clamp(origin + seq.NextOffset(), new_capacity);
      }
      new_ctrl[new_i] = true;
      std::construct_at(std::addressof(new_data[new_i]), std::move(data_[i]));
    }
    ctrl_ = new_ctrl;
    allocator_.deallocate(data_, capacity_);
    data_ = new_data;
    capacity_ *= 2;
  }

  // TODO: ensure that ctrl_.data() and data_ are allocated to a contiguous space in memory
  //   here we hope that the allocator at least puts them on the same page
  std::vector<bool, Allocator> ctrl_;  // invariant: ctrl_.size() == capacity_
  value_type* data_ = nullptr;
  std::size_t size_ = 0;
  std::size_t capacity_ = 1;  // invariant: always a power of 2
  [[no_unique_address]] Allocator allocator_;
};

}  // namespace ty