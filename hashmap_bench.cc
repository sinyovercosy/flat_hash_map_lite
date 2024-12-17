#include <benchmark/benchmark.h>
#include <random>
#include "absl/hash/hash.h"
#include "hashmap.h"

namespace ty {
namespace {

constexpr std::size_t seed = 0x20000524;

template <class M>
void BM_InsertNew(benchmark::State& state) {
  M map;
  const std::size_t init_size = state.range(0);
  for (int i = 0; i < init_size; ++i) {
    // insert a negative number so that keys never match
    benchmark::DoNotOptimize(map.insert({-i, i}));
  }
  std::uniform_int_distribution<int> key_space(/*min=*/1);
  std::default_random_engine prng{seed};
  for (auto _ : state) {
    const int key = key_space(prng);
    benchmark::DoNotOptimize(map.insert({key, key}));
  }
}

template <class M>
void BM_InsertExisting(benchmark::State& state) {
  M map;
  const std::size_t init_size = state.range(0);
  for (int i = 0; i < init_size; ++i) {
    map.insert({i, i});
  }
  std::uniform_int_distribution<int> key_space(0, init_size);
  std::default_random_engine prng{seed};
  for (auto _ : state) {
    const int key = key_space(prng);
    benchmark::DoNotOptimize(map.insert({key, key}));
  }
}

template <class M>
void BM_FindPositive(benchmark::State& state) {
  M map;
  const std::size_t init_size = state.range(0);
  for (int i = 0; i < init_size; ++i) {
    map.insert({i, i});
  }
  std::uniform_int_distribution<int> key_space(0, init_size);
  std::default_random_engine prng{seed};
  for (auto _ : state) {
    const int key = key_space(prng);
    benchmark::DoNotOptimize(map.find(key));
  }
}

template <class M>
void BM_FindNegative(benchmark::State& state) {
  M map;
  const std::size_t init_size = state.range(0);
  for (int i = 0; i < init_size; ++i) {
    // insert a negative number so that keys never match
    benchmark::DoNotOptimize(map.insert({-i, i}));
  }
  std::uniform_int_distribution<int> key_space(/*min=*/1);
  std::default_random_engine prng{seed};
  for (auto _ : state) {
    const int key = key_space(prng);
    benchmark::DoNotOptimize(map.find(key));
  }
}

BENCHMARK(BM_InsertNew<std::unordered_map<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);
BENCHMARK(BM_InsertExisting<std::unordered_map<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);
BENCHMARK(BM_FindPositive<std::unordered_map<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);
BENCHMARK(BM_FindNegative<std::unordered_map<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);

BENCHMARK(BM_InsertNew<flat_hash_map_lite<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);
BENCHMARK(BM_InsertExisting<flat_hash_map_lite<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);
BENCHMARK(BM_FindPositive<flat_hash_map_lite<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);
BENCHMARK(BM_FindNegative<flat_hash_map_lite<int, int, absl::Hash<int>>>)->Range(64, 1 << 20);

}  // namespace
}  // namespace ty