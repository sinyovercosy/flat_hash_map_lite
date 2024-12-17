#include "hashmap.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "gmock/gmock.h"

namespace ty {
namespace {

TEST(MapTest, TestFind) {
  flat_hash_map_lite<std::string, int> map;
  EXPECT_FALSE(map.find("abc"));
  map.insert({"abc", 123});
  auto result = map.find("abc");
  EXPECT_TRUE(result != nullptr);
  EXPECT_EQ(result->second, 123);
  EXPECT_FALSE(map.find("def"));
}

TEST(MapTest, TestInsert) {
  flat_hash_map_lite<std::string, int> map;
  auto result_1 = map.insert({"abc", 123});
  EXPECT_THAT(result_1, ::testing::Pair(::testing::NotNull(), true));
  EXPECT_THAT(*result_1.first, ::testing::Pair("abc", 123));
  auto result_2 = map.insert({"abc", 456});
  EXPECT_THAT(result_2, ::testing::Pair(::testing::NotNull(), false));
  EXPECT_THAT(*result_2.first, ::testing::Pair("abc", 123));
  auto result_3 = map.insert({"def", 123});
  EXPECT_THAT(result_3, ::testing::Pair(testing::Not(result_1.first), true));
  EXPECT_THAT(*result_3.first, ::testing::Pair("def", 123));
}

constexpr auto very_bad_hash = [](std::string const&) noexcept -> std::size_t { return 0; };

TEST(MapTest, TestCollision) {
  flat_hash_map_lite<std::string, int, decltype(very_bad_hash)> map;
  map.insert({"abc", 123});
  map.insert({"def", 456});
  {
    auto result = map.find("abc");
    EXPECT_TRUE(result != nullptr);
    EXPECT_EQ(result->second, 123);
  }
  {
    auto result = map.find("def");
    EXPECT_TRUE(result != nullptr);
    EXPECT_EQ(result->second, 456);
  }
}

}  // namespace
}  // namespace ty
