#include "utility/BCD.h"

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace SiREN {
BCD::BCD(int number, int base) : number_(number), base_(base) {
  if (base == 10) {
    number_ = number;
  }
  if (base == 16) {
    number_ = std::stoi(std::to_string(number), nullptr, 16);
  }
}
BCD::BCD(std::string number, int base) : base_(base) {
  if (base == 10) {
    number_ = std::stoi(number);
  }
  if (base == 16) {
    number_ = std::stoi(number, nullptr, 16);
  }
}
std::string BCD::toString() const {
  std::ostringstream t_oss;
  t_oss << std::setw(2) << std::setfill('0') << number_;
  return t_oss.str();
}
std::int64_t BCD::toInt() const {
  if (number_ < 0) {
    return static_cast<std::uint8_t>(number_);
  }
  if (number_ > 99) {
    return number_ % 100;
  }
  return number_;
}
BCD operator+(const BCD &rbcd, const BCD &lbcd) {
  return BCD(rbcd.toInt() + lbcd.toInt());
}
BCD operator-(const BCD &rbcd, const BCD &lbcd) {
  return BCD(rbcd.toInt() - lbcd.toInt());
}
// BCD operator+(const BCD &rbcd, const std::string &lstr) {
//   BCD result;
//   if (lstr.substr(0, 2) == "0x") {
//     result = rbcd + BCD(std::stoi(lstr, nullptr, 16));
//   } else {
//     result = rbcd + BCD(std::stoi(lstr));
//   }
//   return result;
// }
// BCD operator+(const BCD &rbcd, const int &lbcd) {
//   return BCD(rbcd.toInt() + lbcd);
// }
}  // namespace SiREN