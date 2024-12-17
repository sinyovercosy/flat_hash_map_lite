#include "hashmap.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <random>

namespace ty {
namespace {

using ::testing::Pair;
using ::testing::Pointee;

TEST(MapTest, TestFind) {
  flat_hash_map_lite<std::string, int> map;
  EXPECT_EQ(map.find("abc"), nullptr);
  map.insert({"abc", 123});
  EXPECT_THAT(map.find("abc"), Pointee(Pair("abc", 123)));
  EXPECT_EQ(map.find("def"), nullptr);
}

TEST(MapTest, TestInsert) {
  flat_hash_map_lite<std::string, int> map;
  {
    auto [it, inserted] = map.insert({"abc", 123});
    EXPECT_TRUE(inserted);
    EXPECT_THAT(*it, Pair("abc", 123));
  }
  {
    auto [it, inserted] = map.insert({"abc", 456});
    EXPECT_FALSE(inserted);
    EXPECT_THAT(*it, Pair("abc", 123));
  }
  {
    auto [it, inserted] = map.insert({"def", 789});
    EXPECT_TRUE(inserted);
    EXPECT_THAT(*it, Pair("def", 789));
  }
}

constexpr auto very_bad_hash = [](std::string const&) noexcept -> std::size_t { return 0; };

TEST(MapTest, TestCollision) {
  flat_hash_map_lite<std::string, int, decltype(very_bad_hash)> map;
  map.insert({"abc", 123});
  map.insert({"def", 456});
  EXPECT_THAT(map.find("abc"), Pointee(Pair("abc", 123)));
  EXPECT_THAT(map.find("def"), Pointee(Pair("def", 456)));
}

constexpr auto very_bad_eq = [](std::string const&, std::string const&) noexcept { return true; };

TEST(MapTest, TestEq) {
  flat_hash_map_lite<std::string, int, decltype(very_bad_hash), decltype(very_bad_eq)> map;
  {
    auto [it, inserted] = map.insert({"abc", 123});
    EXPECT_TRUE(inserted);
    EXPECT_THAT(*it, Pair("abc", 123));
  }
  {
    auto [it, inserted] = map.insert({"def", 456});
    EXPECT_FALSE(inserted);
    EXPECT_THAT(*it, Pair("abc", 123));
  }
  {
    auto it = map.find("ghi");
    EXPECT_TRUE(it != nullptr);
    EXPECT_THAT(*it, Pair("abc", 123));
  }
}

TEST(MapTest, FuzzTest) {
  std::default_random_engine prng{std::random_device{}()};
  std::uniform_int_distribution<int> key_space(-100, 100);
  std::uniform_int_distribution<int> value_space;
  std::unordered_map<int, int> std_map;
  flat_hash_map_lite<int, int> ty_map;
  for (int i = 0; i < 1'000'000; ++i) {
    const int key = key_space(prng);
    const int val = value_space(prng);
    auto [std_it, std_inserted] = std_map.insert({key, val});
    auto [ty_it, ty_inserted] = ty_map.insert({key, val});
    ASSERT_EQ(std_inserted, ty_inserted);
    ASSERT_TRUE(std_it != std_map.end());
    ASSERT_TRUE(ty_it != nullptr);
    ASSERT_EQ(*std_it, *ty_it);
  }

  for (int i = 0; i < 1'000'000; ++i) {
    const int key = key_space(prng);
    auto std_it = std_map.find(key);
    auto ty_it = ty_map.find(key);
    ASSERT_EQ(std_it == std_map.end(), ty_it == nullptr);
    if (std_it != std_map.end()) {
      ASSERT_EQ(*std_it, *ty_it);
    }
  }
}

}  // namespace
}  // namespace ty
