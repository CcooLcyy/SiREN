#include <gtest/gtest.h>

#include <cstdint>

#include "DLT645.h"
// #include "DLT645ConfigBuilder.h"
#include "DLT645DataSheet.h"

namespace siren {
namespace dlt645 {
class TestDLT645 : public ::testing::Test {
protected:
  void SetUp() override {
    obj = std::make_unique<DLT645>("000000000000", "test_data_sheet.json");
  }

  std::unique_ptr<DLT645> obj;
};
TEST(TestDLT645, genDLT645DataSheet) {
  DLT645DataSheet dataSheet;
}
// TEST(TestDLT645, getDLT645Obj) {
//   DLT645ConfigBuilder builder1;
//   builder1.setAddr("123456789012").setDataSheet("").final();
//   DLT645 dlt645;
//   dlt645.setConfig(builder1);
//   dlt645.getConfig();

//   DLT645ConfigBuilder builder2;
//   builder2.setAddr("").setDataSheetPath("").final();
//   DLT645 dlt645;
//   dlt645.setConfig(builder1);
// }
TEST_F(TestDLT645, extractDlt645Message) {
  const std::vector<std::uint8_t> message = obj->extractDlt645Message({0x00});
  // EXPECT_EQ(, );
}
}  // namespace dlt645
}  // namespace siren