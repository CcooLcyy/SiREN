#pragma once

#include <cstdint>
#include <string>

namespace SiREN {
class BCD {
  public:
  friend BCD operator+(const BCD &rbcd, const BCD &lbcd);
  friend BCD operator-(const BCD &rbcd, const BCD &lbcd);
  friend BCD operator+(const BCD &rbcd, const int &lbcd);

  BCD() = default;
  BCD(int number, int base = 10);
  BCD(std::string number, int base = 10);
  ~BCD() = default;

  std::string toString() const;
  std::int64_t toInt() const;

  protected:
  std::int64_t number_;
  int base_;
};
BCD operator+(const BCD &rbcd, const BCD &lbcd);
BCD operator-(const BCD &rbcd, const BCD &lbcd);
BCD operator+(const BCD &rbcd, const std::string &lstr);
}  // namespace SiREN