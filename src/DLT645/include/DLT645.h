#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest_prod.h>

#include "DLT645.pb.h"

namespace siren {
namespace dlt645 {
struct DataSheetCache;

class DLT645 {
public:
  DLT645() = delete;
  // Dlt645(std::string address);
  DLT645(std::string address, std::string dataSheetPath);
  ~DLT645();

  Dlt645Proto::DeviceData getDeviceData();
  std::string resolveBlockName(const std::string &model) const;
  const std::string &lastDecodeError() const;

  Dlt645Proto::DeviceData decodeRecvReadMeter(std::vector<std::uint8_t> rowData);
  std::vector<std::uint8_t> encodeSendReadMeter(Dlt645Proto::Data data);
  std::vector<std::uint8_t> encodeSendWriteMeter(Dlt645Proto::Data data);
  std::vector<std::uint8_t> encodeSendCtrlMeter(Dlt645Proto::Data data);

  // 提取有效的DLT645报文
  const std::vector<std::uint8_t> extractDlt645Message(std::vector<std::uint8_t> rowData);

protected:
  void getDataSheet(std::string dataSheetPath);
  std::uint8_t encodeCS(const std::vector<std::uint8_t> &buffer);
  std::uint8_t toBCD(std::uint8_t code);
  std::uint8_t fromBCD(std::uint8_t code);

private:
  static void clearDataSheetCacheForTest();
  static std::size_t getDataSheetCacheSizeForTest();

  FRIEND_TEST(TestDLT645, ReusesDataSheetCacheByPath);
  FRIEND_TEST(TestDLT645, ResolveBlockNameUsesCachedIndex);
  FRIEND_TEST(TestDLT645, DecodeRecvReadMeterUsesCachedBlockTemplate);
  FRIEND_TEST(TestDLT645, DecodeRecvReadMeterRecordsFailureReason);

  std::size_t diSize_;
  std::size_t dataSize_{0};
  std::string address_;
  std::string dataSheetPath_;
  std::shared_ptr<const DataSheetCache> dataSheetCache_;
  std::string lastDecodeError_;
};
}  // namespace dlt645
}  // namespace siren
