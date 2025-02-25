#include "utility/RandomGen.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <regex>
#include <unordered_set>
#include <vector>

TEST(RandomGen, UUID) {
  std::regex pattern("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$");
  EXPECT_TRUE(std::regex_match(SiREN::RandomGen::UUID(), pattern));
}
TEST(RandomGen, randomNum) {
  std::vector<std::uint64_t> randomNumVec;
  for (auto i = 0; i != 10000; i++) {
    randomNumVec.emplace_back(SiREN::RandomGen::randomNum());
  }
  std::unordered_set<std::uint64_t> t_set;
  EXPECT_TRUE([&]() {
    bool result;
    for (const auto &num : randomNumVec) {
      if (t_set.find(num) == t_set.end()) {
        t_set.insert(num);
        result = true;
      } else {
        result = false;
      }
    }
    return result;
  }());
  EXPECT_EQ(randomNumVec.size(), 10000);
}