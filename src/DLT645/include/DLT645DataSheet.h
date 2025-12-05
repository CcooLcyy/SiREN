#pragma once
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace siren {
namespace dlt645 {
namespace _impl {
struct DataSheet_data_dataParse_binList {
  std::uint64_t set;
  std::string binName;
  std::string binRealName;
  std::string binValue;
  std::vector<std::uint64_t> binUnion;
};
struct DataSheet_data_dataParse {
  std::uint64_t number;
  std::string point;
  std::string name;
  std::string realName;
  std::uint64_t dataSize;
  std::double_t factor;
  std::string encodType;
  std::string dataValue;
};
struct DataSheet_data {
  std::string blockName;
  std::string blockPoint;
  std::string blockRealName;
};
struct DataSheet {
  std::string deviceName;
  std::string deviceAddress;
  std::vector<DataSheet_data> data;
};
}  // namespace _impl

class DLT645DataSheet {
  friend class TestDLT645DataSheet;

public:
  DLT645DataSheet();
  ~DLT645DataSheet();

private:
  _impl::DataSheet dataSheet_;
};

}  // namespace dlt645
}  // namespace siren