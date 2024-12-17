#include <cassert>
#include <concepts>
#include <memory>
#include <vector>

namespace ty {

template <typename K, typename V, typename Hash = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  requires std::equality_comparable<K> && std::is_nothrow_invocable_r_v<std::size_t, Hash, K> && std::is_move_constructible_v<std::pair<const K, V>>
class flat_hash_map_lite {
 public:
  using value_type = std::pair<const K, V>;

  bool empty() const { return size_ == 0; }

  // TODO: 1) return an iterator, and 2) add const version
  value_type* find(K const& key) {
    if (empty()) {
      return nullptr;  // special case where Capacity() == 1 but we have not allocated anything
    }
    ProbeSeq seq;
    const std::size_t origin = Clamp(Hash{}(key), capacity_);
    std::size_t i = origin;
    while (ctrl_[i]) {
      if (data_[i].first == key) {
        return std::addressof(data_[i]);
      } else {
        i = Clamp(origin + seq.NextOffset(), capacity_);
      }
    }
    return nullptr;
  }

  void insert(value_type kv) {
    size_++;
    if (size_ >= ResizeThreshold()) {
      Rehash();
    }
    assert(size_ < capacity_);  // always exists a free slot
    std::size_t i = ProbeNextFreeSlot(kv.first, ctrl_);
    ctrl_[i] = true;
    std::construct_at(std::addressof(data_[i]), std::move(kv));
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

  std::size_t ProbeNextFreeSlot(K const& key, std::vector<bool, Allocator> const& not_free) {
    const std::size_t origin = Clamp(Hash{}(key), not_free.size());
    ProbeSeq seq;
    std::size_t i = origin;
    while (not_free[i]) {
      i = Clamp(origin + seq.NextOffset(), not_free.size());
    }
    return i;
  }

  void Rehash() {
    std::vector<bool, Allocator> new_ctrl(capacity_ * 2, allocator_);
    value_type* new_data = allocator_.allocate(capacity_ * 2);
    for (std::size_t i = 0; i < ctrl_.size(); ++i) {
      if (!ctrl_[i]) {
        continue;
      }
      std::size_t new_i = ProbeNextFreeSlot(data_[i].first, new_ctrl);
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