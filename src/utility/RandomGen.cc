#include "utility/RandomGen.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdint>
#include <random>

namespace SiREN {
std::string RandomGen::UUID() {
  return boost::uuids::to_string(boost::uuids::random_generator()());
}
std::uint64_t RandomGen::randomNum() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 1410065417);
  return distrib(gen);
}
}  // namespace SiREN