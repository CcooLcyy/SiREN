#pragma once
#include <cstdint>
#include <string>

namespace SiREN {
class RandomGen {
  public:
  RandomGen();
  ~RandomGen();

  static std::string UUID();
  static std::uint64_t randomNum();

  private:
};
}  // namespace SiREN