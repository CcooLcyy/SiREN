#include "utility/BCD.h"

#include <gtest/gtest.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

TEST(BCD, toString) {
  for (int i = 0; i != 100; i++) {
    auto result = std::ostringstream() << std::setw(2) << std::setfill('0') << i;
    EXPECT_EQ(SiREN::BCD(i).toString(), result.str());
  }
}
TEST(BCD, toInt) {
  for (int i = 0; i != 100; i++) {
    std::ostringstream tOss;
    tOss << i;
    EXPECT_EQ(SiREN::BCD(i).toInt(), i);
    EXPECT_EQ(SiREN::BCD(tOss.str()).toInt(), i);
  }
}
TEST(BCD, operator_BCD_add_BCD) {
  EXPECT_EQ((SiREN::BCD(10) + SiREN::BCD(91)).toInt(), 1);
  EXPECT_EQ((SiREN::BCD("16", 16) + SiREN::BCD("15")).toInt(), 37);
  EXPECT_EQ((SiREN::BCD("50") + SiREN::BCD("51")).toInt(), 1);
}
TEST(BCD, operator_BCD_minus_BCD) {
  EXPECT_EQ((SiREN::BCD("32", 16) - SiREN::BCD("33", 16)).toInt(), 0xff);
}