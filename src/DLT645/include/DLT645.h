#pragma once

#include "DLT645.pb.h"

class Dlt645 {
public:
  Dlt645() = delete;
  // Dlt645(std::string address);
  Dlt645(std::string address, std::string dataSheetPath);
  ~Dlt645();

  Dlt645Proto::DeviceData getDeviceData();

  Dlt645Proto::DeviceData decodeRecvReadMeter(std::vector<std::uint8_t> rowData);
  std::vector<std::uint8_t> encodeSendReadMeter(Dlt645Proto::Data data);
  std::vector<std::uint8_t> encodeSendWriteMeter(Dlt645Proto::Data data);
  std::vector<std::uint8_t> encodeSendCtrlMeter(Dlt645Proto::Data data);

private:
  void getDataSheet(std::string dataSheetPath);
  std::uint8_t encodeCS(const std::vector<std::uint8_t> &buffer);
  std::uint8_t toBCD(std::uint8_t code);
  std::uint8_t fromBCD(std::uint8_t code);
  // std::string toACSII(std::uint8_t code);
  // std::uint8_t fromACSII(std::string ascii);

  std::size_t diSize_;
  std::size_t dataSize_{0};
  std::string jsonContext_;
  std::string address_;
  // std::string dataSheetPath_;
};