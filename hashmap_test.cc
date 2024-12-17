#include "hashmap.h"
#include <gtest/gtest.h>

namespace ty {
namespace {

TEST(MapTest, TestBasic) {
  flat_hash_map_lite<std::string, int> map;
  EXPECT_FALSE(map.find("abc"));
  map.insert({"abc", 123});
  auto result = map.find("abc");
  EXPECT_TRUE(result != nullptr);
  EXPECT_EQ(result->second, 123);
  EXPECT_FALSE(map.find("def"));
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
