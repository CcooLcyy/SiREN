#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <deque>

#include "DLT645.h"

namespace siren {
namespace dlt645 {
namespace {

std::filesystem::path makeTempDataSheetPath(const std::string &fileName) {
  return std::filesystem::temp_directory_path() / fileName;
}

void writeDataSheet(const std::filesystem::path &path, const std::string &deviceName) {
  std::ofstream ofs(path);
  ofs << "{\n"
         "  \"deviceName\": \""
      << deviceName
      << "\",\n"
         "  \"data\": [\n"
         "    {\n"
         "      \"blockName\": \"BLK_A\",\n"
         "      \"blockPoint\": \"04030201\",\n"
         "      \"dataParse\": [\n"
         "        {\n"
         "          \"name\": \"ModelA\",\n"
         "          \"dataSize\": 2,\n"
         "          \"factor\": 0.1,\n"
         "          \"encodeType\": \"BCD\",\n"
         "          \"dataType\": \"double\"\n"
         "        }\n"
         "      ]\n"
         "    },\n"
         "    {\n"
         "      \"blockName\": \"BLK_B\",\n"
         "      \"blockPoint\": \"44332211\",\n"
         "      \"dataParse\": [\n"
         "        {\n"
         "          \"name\": \"StatusRaw\",\n"
         "          \"dataSize\": 1,\n"
         "          \"factor\": 1,\n"
         "          \"encodeType\": \"BIN\",\n"
         "          \"dataType\": \"string\",\n"
         "          \"binList\": [\n"
         "            {\"set\": 0, \"binName\": \"SwitchOn\"},\n"
         "            {\"set\": 1, \"binName\": \"SwitchOff\"}\n"
         "          ]\n"
         "        }\n"
         "      ]\n"
         "    }\n"
         "  ]\n"
         "}\n";
}

std::vector<std::uint8_t> makeReadReplyFrame(const std::string &blockPoint, const std::vector<std::uint8_t> &payload) {
  std::deque<std::uint8_t> dataIdent;
  for (std::size_t i = 0; i < blockPoint.size(); i += 2) {
    dataIdent.emplace_front(static_cast<std::uint8_t>(std::stoi(blockPoint.substr(i, 2), nullptr, 16)));
  }
  for (auto &byte : dataIdent) {
    byte += 0x33;
  }

  std::vector<std::uint8_t> frame = {
      0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x91, static_cast<std::uint8_t>(dataIdent.size() + payload.size())};
  frame.insert(frame.end(), dataIdent.begin(), dataIdent.end());
  for (auto byte : payload) {
    frame.emplace_back(static_cast<std::uint8_t>(byte + 0x33));
  }

  std::uint8_t cs = 0;
  for (auto byte : frame) {
    cs += byte;
  }
  frame.emplace_back(cs);
  frame.emplace_back(0x16);
  return frame;
}

class DataSheetFileGuard {
public:
  explicit DataSheetFileGuard(std::filesystem::path path) :
    path_(std::move(path)) {}

  ~DataSheetFileGuard() {
    std::error_code ec;
    std::filesystem::remove(path_, ec);
  }

  const std::filesystem::path &path() const {
    return path_;
  }

private:
  std::filesystem::path path_;
};

}  // namespace

TEST(TestDLT645, ReusesDataSheetCacheByPath) {
  DLT645::clearDataSheetCacheForTest();
  DataSheetFileGuard dataSheetPath(makeTempDataSheetPath("dlt645_cache_reuse_test.json"));
  writeDataSheet(dataSheetPath.path(), "DeviceAlpha");

  DLT645 first("001122334455", dataSheetPath.path().string());
  auto firstData = first.getDeviceData();
  EXPECT_EQ(firstData.devicename(), "DeviceAlpha");
  EXPECT_EQ(firstData.deviceaddress(), "001122334455");

  writeDataSheet(dataSheetPath.path(), "DeviceBeta");

  DLT645 second("667788990011", dataSheetPath.path().string());
  auto secondData = second.getDeviceData();
  EXPECT_EQ(secondData.devicename(), "DeviceAlpha");
  EXPECT_EQ(secondData.deviceaddress(), "667788990011");
  EXPECT_EQ(DLT645::getDataSheetCacheSizeForTest(), 1U);
}

TEST(TestDLT645, ResolveBlockNameUsesCachedIndex) {
  DLT645::clearDataSheetCacheForTest();
  DataSheetFileGuard dataSheetPath(makeTempDataSheetPath("dlt645_model_index_test.json"));
  writeDataSheet(dataSheetPath.path(), "DeviceAlpha");

  DLT645 dlt645("001122334455", dataSheetPath.path().string());
  EXPECT_EQ(dlt645.resolveBlockName("ModelA"), "BLK_A");
  EXPECT_EQ(dlt645.resolveBlockName("SwitchOn"), "BLK_B");
  EXPECT_EQ(dlt645.resolveBlockName("UnknownModel"), "UnknownModel");
}

TEST(TestDLT645, DecodeRecvReadMeterUsesCachedBlockTemplate) {
  DLT645::clearDataSheetCacheForTest();
  DataSheetFileGuard dataSheetPath(makeTempDataSheetPath("dlt645_decode_block_test.json"));
  writeDataSheet(dataSheetPath.path(), "DeviceAlpha");

  DLT645 dlt645("001122334455", dataSheetPath.path().string());
  const auto frame = makeReadReplyFrame("04030201", {0x34, 0x12});
  const auto parsed = dlt645.decodeRecvReadMeter(frame);

  ASSERT_EQ(parsed.data_size(), 1);
  EXPECT_EQ(parsed.devicename(), "DeviceAlpha");
  EXPECT_EQ(parsed.data(0).blockname(), "BLK_A");
  ASSERT_EQ(parsed.data(0).dataparse_size(), 1);
  EXPECT_EQ(parsed.data(0).dataparse(0).name(), "ModelA");
  EXPECT_NEAR(std::stod(parsed.data(0).dataparse(0).datavalue()), 123.4, 1e-6);
}

TEST(TestDLT645, DecodeRecvReadMeterRecordsFailureReason) {
  DLT645::clearDataSheetCacheForTest();
  DataSheetFileGuard dataSheetPath(makeTempDataSheetPath("dlt645_decode_reason_test.json"));
  writeDataSheet(dataSheetPath.path(), "DeviceAlpha");

  DLT645 dlt645("001122334455", dataSheetPath.path().string());
  const auto frame = makeReadReplyFrame("04030201", {0x34});
  const auto parsed = dlt645.decodeRecvReadMeter(frame);

  EXPECT_EQ(parsed.data_size(), 0);
  EXPECT_EQ(dlt645.lastDecodeError(), "数据长度不足: name=ModelA, need=2, remain=1");
}

}  // namespace dlt645
}  // namespace siren
