#include "DLT645.h"

#include <absl/status/status.h>
#include <google/protobuf/json/json.h>
#include <google/protobuf/util/json_util.h>

#include <algorithm>
#include <boost/json/object.hpp>
#include <boost/json/parser.hpp>
#include <boost/json/stream_parser.hpp>
#include <cctype>
#include <deque>
#include <fstream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

#include "CtrlCode.h"
#include "DLT645.pb.h"
#include "DLT645Public.h"
#include "Logger.h"

namespace siren {
namespace dlt645 {
DLT645::DLT645(std::string address, std::string dataSheetPath) {
  diSize_ = DI_SIZE;
  address_ = address;
  getDataSheet(dataSheetPath);
}
DLT645::~DLT645() {}
Dlt645Proto::DeviceData DLT645::getDeviceData() {
  Dlt645Proto::DeviceData result;
  if (address_ == "") {
  }
  google::protobuf::util::JsonParseOptions option;
  option.ignore_unknown_fields = true;
  auto status = google::protobuf::util::JsonStringToMessage(jsonContext_, &result, option);
  if (status != absl::OkStatus()) {
    SIREN_LOG_ERROR << "dataSheet解析错误: " << status.ToString();
  }
  result.set_deviceaddress(address_);
  return result;
}
Dlt645Proto::DeviceData DLT645::decodeRecvReadMeter(std::vector<std::uint8_t> rowData) {
  Dlt645Proto::DeviceData message;
  if (rowData.size() == 0) {
    return message;
  }
  if (rowData.front() != 0x68) {
  }
  if (rowData.size() < SECOND_START_FLAG)
    return Dlt645Proto::DeviceData();

  if (rowData.at(SECOND_START_FLAG) != 0x68) {
  }
  std::size_t dataSize = rowData.at(DATA_SIZE_POS);
  std::size_t dataBegin = 7 + diSize_ + 2;
  if (dataSize < 4) {
    return Dlt645Proto::DeviceData();
  }
  std::deque<std::uint8_t> dataIdentDeq = std::deque<std::uint8_t>(rowData.begin() + 10, rowData.begin() + 10 + diSize_);

  std::vector<std::uint8_t> rowDataItem = std::vector<std::uint8_t>(rowData.begin() + 10 + diSize_, rowData.end() - 2);
  for (auto &data : rowDataItem) {
    data -= 0x33;
  }
  std::string dataIdentStr;
  for (auto it = dataIdentDeq.rbegin(); it != dataIdentDeq.rend(); ++it) {
    std::ostringstream t_oss;
    t_oss << std::hex << std::setw(2) << std::setfill('0') << ((*it - 0x33) & 0xff);
    dataIdentStr += t_oss.str();
  }
  google::protobuf::util::JsonParseOptions option;
  option.ignore_unknown_fields = true;
  auto status = google::protobuf::json::JsonStringToMessage(jsonContext_, &message, option);
  if (status != absl::OkStatus()) {
    throw std::runtime_error("转换失败: " + status.ToString());
  }
  for (auto &data : *message.mutable_data()) {
    if (data.blockpoint() == dataIdentStr) {
      std::size_t begin = 0;
      std::size_t end = 0;
      for (auto &dataParse : *data.mutable_dataparse()) {
        bool neg{false};
        begin = end;
        end = dataParse.datasize() + begin;
        std::deque<std::uint8_t> dataDeq = std::deque<std::uint8_t>(rowDataItem.begin() + begin, rowDataItem.begin() + end);
        if (*(dataDeq.end() - 1) & 0x80) {
          auto lastData = *(dataDeq.end() - 1);
          dataDeq.pop_back();
          dataDeq.push_back(lastData ^ 0x80);
          neg = true;
        }
        std::string realData;
        for (auto rit = dataDeq.rbegin(); rit != dataDeq.rend(); rit++) {
          std::ostringstream t_oss;
          t_oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*rit);
          realData += t_oss.str();
        }
        std::string dataValueStr;
        if (dataParse.encodetype() == "BCD") {
          auto dataValue = std::stod(realData) * dataParse.factor();
          dataValueStr = std::to_string(std::stod(realData) * dataParse.factor());
          if (neg) {
            dataValueStr = "-" + dataValueStr;
          }
          if (dataParse.datatype() == "int") {
            dataValueStr = std::to_string(std::stoi(dataValueStr));
          } else if (dataParse.datatype() == "string") {
            // 应对BCD编码但是数据类型为string的情况
            // 针对不以ASCII进行编码的string
            dataValueStr = std::to_string(std::stoi(dataValueStr));
            std::ostringstream oss;
            oss << std::setfill('0') << std::setw(dataParse.datasize() * 2) << dataValueStr;
            dataValueStr = oss.str();

            // 当BCD数据类型位string但是dataTranslate位置非空
            // 表示报文解到的数据需要映射为显示世界的物理量。
            if (dataParse.datatranslate_size() != 0) {
              auto dataTranslateIt = std::find_if(dataParse.datatranslate().begin(), dataParse.datatranslate().end(), [&](const auto &elem) {
                return elem.rowdata() == dataValueStr;
              });
              if (dataTranslateIt != dataParse.datatranslate().end()) {
                dataValueStr = dataTranslateIt->translatedata();
              }
            }
          }
          dataParse.set_datavalue(dataValueStr);
        } else if (dataParse.encodetype() == "BIN") {
          auto binValue = std::stoi(realData, nullptr, 16);
          for (auto &bin : *dataParse.mutable_binlist()) {
            // binunion作为合点出现，如果需要多个点表示一个信息在这个项中填写。
            if (bin.binunion().empty()) {
              auto mask = (1 << bin.set());
              if ((binValue & mask) == mask) {
                bin.set_binvalue("1");
              } else {
                bin.set_binvalue("0");
              }
            } else {
              std::uint64_t mask = 0;
              for (auto unionSet : bin.binunion()) {
                mask |= (1 << unionSet);
              }
            }
            // 如何bin格式数据类型为string，则将被置位的位名设定为对应的值
            if (dataParse.datatype() == "string") {
              for (auto bin : dataParse.binlist()) {
                if (bin.binvalue() == "1") {
                  dataParse.set_datavalue(bin.binname());
                }
              }
            } else {
              dataParse.set_datavalue(bin.binvalue());
            }
          }
        } else if (dataParse.encodetype() == "ASCII") {
          std::deque<char> realDataDeq;
          for (int i = 0; i < realData.size(); i += 2) {
            char c = static_cast<char>(std::stoi(realData.substr(i, 2), nullptr, 16));
            if (c != '\0') {
              realDataDeq.emplace_back(c);
            }
          }
          for (auto str : realDataDeq) {
            dataValueStr += str;
          }
          dataParse.set_datavalue(dataValueStr);
        }

        // 处理数据转换的情况（dataTranslate）
        if (dataParse.datatranslate_size() != 0) {
          for (auto datatranslate : dataParse.datatranslate()) {
            if (dataParse.datavalue() == datatranslate.rowdata()) {
              dataParse.set_datavalue(datatranslate.translatedata());
            }
          }
        }
      }
    }
  }
  return message;
}
void DLT645::getDataSheet(std::string dataSheetPath) {
  std::ifstream dataSheetFile(dataSheetPath);
  if (!dataSheetFile.is_open()) {
    throw std::runtime_error("打开数据解析列表失败");
  }
  jsonContext_ = std::string((std::istreambuf_iterator<char>(dataSheetFile)), std::istreambuf_iterator<char>());
  dataSheetFile.close();
}
std::vector<std::uint8_t> DLT645::encodeSendReadMeter(Dlt645Proto::Data data) {
  if (address_ == "") {
  }
  auto dataPointStr = data.blockpoint();
  std::deque<std::uint8_t> dataPoint;
  for (int i = 0; i != dataPointStr.size(); i += 2) {
    dataPoint.emplace_front(std::stoi(dataPointStr.substr(i, 2), nullptr, 16));
  }
  std::for_each(dataPoint.begin(), dataPoint.end(), [&](std::uint8_t &elem) {
    elem += 0x33;
  });
  std::deque<std::uint8_t> address;
  for (int i = 0; i != address_.size(); i += 2) {
    address.emplace_front(std::stoi(address_.substr(i, 2), nullptr, 16));
  }
  std::vector<std::uint8_t> result;
  result.emplace_back(0x68);
  result.insert(result.end(), address.begin(), address.end());
  result.emplace_back(0x68);
  result.emplace_back(static_cast<std::uint8_t>(CtrlCode::FuncCode::READ_DATA));
  result.emplace_back(diSize_ + dataSize_);
  result.insert(result.end(), dataPoint.begin(), dataPoint.end());
  result.emplace_back(encodeCS(result));
  result.emplace_back(0x16);
  return result;
}
std::vector<std::uint8_t> DLT645::encodeSendWriteMeter(Dlt645Proto::Data data) {
  if (address_ == "") {
  }
  std::vector<std::uint8_t> result;
  if (data.dataparse_size() != 1) {
    // Logger::error("dataParse数量错误，不返回任何报文");
    return result;
  }
  auto dataPointStr = data.blockpoint();
  std::deque<std::uint8_t> dataPoint;
  for (int i = 0; i != dataPointStr.size(); i += 2) {
    dataPoint.emplace_front(std::stoi(dataPointStr.substr(i, 2), nullptr, 16));
  }
  std::for_each(dataPoint.begin(), dataPoint.end(), [&](std::uint8_t &elem) {
    elem += 0x33;
  });
  std::deque<std::uint8_t> address;
  for (int i = 0; i != address_.size(); i += 2) {
    address.emplace_front(std::stoi(address_.substr(i, 2), nullptr, 16));
  }
  dataSize_ = data.dataparse().begin()->datasize();
  result.emplace_back(0x68);
  result.insert(result.end(), address.begin(), address.end());
  result.emplace_back(0x68);
  result.emplace_back(0x14);                     // 主站下发写数据
  result.emplace_back(diSize_ + dataSize_ + 8);  // 数据长度
  result.insert(result.end(), dataPoint.begin(), dataPoint.end());
  // 全0表示操作者代码和密码
  std::vector<std::uint8_t> allZero(8, 0x33);
  result.insert(result.end(), allZero.begin(), allZero.end());
  {
    auto dataParse = data.dataparse().begin();
    if (dataParse->encodetype() == "BCD") {
      // 添加数据
      auto tmpData = std::stoi(data.dataparse().begin()->datavalue());
      std::ostringstream oss;
      oss << std::setw(data.dataparse().begin()->datasize() * 2) << std::setfill('0') << tmpData / data.dataparse().begin()->factor();
      std::deque<std::string> tmpDeq;
      for (int i = 0; i != oss.str().size(); i += 2) {
        tmpDeq.emplace_front(oss.str().substr(i, 2));
      }
      for (auto str : tmpDeq) {
        result.emplace_back(std::stoi(str, nullptr, 16) + 0x33);
      }
    } else if (dataParse->encodetype() == "BIN") {
      if (dataParse->datatype() == "string") {
        // 如果BIN编码为string则将dataValue的值与binName的值匹配，得到set进行发送
        for (auto bin : dataParse->binlist()) {
          if (bin.binname() == dataParse->datavalue()) {
            auto byte = 0;
            byte |= (1 << bin.set());
            result.emplace_back(byte + 0x33);
          }
        }
      } else if (dataParse->datatype() == "int") {
        // 如果BIN编码位int则直接将dataValue的值发送
        result.emplace_back(std::stoi(dataParse->datavalue()) + 0x33);
      }
    }
  }
  result.emplace_back(encodeCS(result));
  result.emplace_back(FRAME_END);
  return result;
}
std::vector<std::uint8_t> DLT645::encodeSendCtrlMeter(Dlt645Proto::Data data) {
  if (address_ == "") {
  }
  std::vector<std::uint8_t> result;
  if (data.dataparse_size() != 1) {
    // Logger::error("dataParse数量错误，不返回任何报文");
    return result;
  }
  auto dataPointStr = data.blockpoint();
  std::deque<std::uint8_t> dataPoint;
  for (int i = 0; i != dataPointStr.size(); i += 2) {
    dataPoint.emplace_front(std::stoi(dataPointStr.substr(i, 2), nullptr, 16));
  }
  std::for_each(dataPoint.begin(), dataPoint.end(), [&](std::uint8_t &elem) {
    elem += 0x33;
  });
  std::deque<std::uint8_t> address;
  for (int i = 0; i != address_.size(); i += 2) {
    address.emplace_front(std::stoi(address_.substr(i, 2), nullptr, 16));
  }
  dataSize_ = data.dataparse().begin()->datasize();
  result.emplace_back(0x68);
  result.insert(result.end(), address.begin(), address.end());
  result.emplace_back(0x68);
  result.emplace_back(0x1c);                     // 主站下发控制
  result.emplace_back(diSize_ + dataSize_ + 8);  // 数据长度
  result.insert(result.end(), dataPoint.begin(), dataPoint.end());
  // 全0表示操作者代码和密码
  std::vector<std::uint8_t> allZero(8, 0x33);
  result.insert(result.end(), allZero.begin(), allZero.end());
  {
    // 添加数据
    auto tmpData = std::stoi(data.dataparse().begin()->datavalue());
    std::ostringstream oss;
    oss << std::setw(data.dataparse().begin()->datasize() * 2) << std::setfill('0') << tmpData / data.dataparse().begin()->factor();
    std::deque<std::string> tmpDeq;
    for (int i = 0; i != oss.str().size(); i += 2) {
      tmpDeq.emplace_front(oss.str().substr(i, 2));
    }
    for (auto str : tmpDeq) {
      result.emplace_back(std::stoi(str, nullptr, 16) + 0x33);
    }
  }
  result.emplace_back(encodeCS(result));
  result.emplace_back(FRAME_END);
  return result;
}
std::uint8_t DLT645::encodeCS(const std::vector<std::uint8_t> &buffer) {
  std::uint8_t result = 0;
  for (auto byte : buffer) {
    result += byte;
  }
  return result & 0xff;
}
std::uint8_t DLT645::toBCD(std::uint8_t code) {
  std::uint8_t high = code / 10;
  std::uint8_t low = code % 10;
  return (high << 4) | low;
}
std::uint8_t DLT645::fromBCD(std::uint8_t code) {
  std::uint8_t high = (code >> 4) & 0x0F;
  std::uint8_t low = code & 0x0F;
  return high * 10 + low;
}
const std::vector<std::uint8_t> DLT645::extractDlt645Message(std::vector<std::uint8_t> rowData) {
  return std::vector<std::uint8_t>();
}
}  // namespace dlt645
}  // namespace siren