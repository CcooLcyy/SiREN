#pragma once

#include <gtest/gtest_prod.h>

#include "DLT645.pb.h"

namespace siren {
class DLT645 {
public:
  DLT645() = delete;
  // Dlt645(std::string address);
  DLT645(std::string address, std::string dataSheetPath);
  ~DLT645();

  Dlt645Proto::DeviceData getDeviceData();

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

  std::size_t diSize_;
  std::size_t dataSize_{0};
  std::string jsonContext_;
  std::string address_;
};
}  // namespace siren