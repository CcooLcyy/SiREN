#include "DLT645.h"

#include <absl/status/status.h>
#include <google/protobuf/util/json_util.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <deque>
#include <fstream>
#include <iomanip>
#include <ios>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "CtrlCode.h"
#include "DLT645.pb.h"
#include "DLT645Public.h"
#include "Logger.h"

namespace siren {
namespace dlt645 {
namespace {

std::string toUpperAscii(std::string value) {
  for (auto &ch : value) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  }
  return value;
}

std::shared_ptr<const DataSheetCache> loadDataSheetCache(const std::string &dataSheetPath);

bool hasOnlyTrailingWhitespace(const std::string &text, std::size_t pos) {
  for (; pos < text.size(); ++pos) {
    if (!std::isspace(static_cast<unsigned char>(text[pos]))) {
      return false;
    }
  }
  return true;
}

std::vector<std::uint8_t> encodeScaledBcdDataBytes(const Dlt645Proto::DataParse &dataParse, std::string &error) {
  error.clear();
  if (dataParse.datasize() <= 0) {
    error = "BCD编码dataSize非法: name=" + dataParse.name();
    return {};
  }
  const auto dataSize = static_cast<std::size_t>(dataParse.datasize());
  if (dataSize > std::numeric_limits<std::size_t>::max() / 2) {
    error = "BCD编码dataSize过大: name=" + dataParse.name();
    return {};
  }

  const auto factor = dataParse.factor();
  if (!std::isfinite(factor) || factor <= 0.0) {
    error = "BCD编码factor非法: name=" + dataParse.name();
    return {};
  }

  double value = 0.0;
  std::size_t parsedSize = 0;
  try {
    value = std::stod(dataParse.datavalue(), &parsedSize);
  } catch (const std::exception &e) {
    error = "BCD编码值非法: name=" + dataParse.name() + ", value=" + dataParse.datavalue() + ", reason=" + e.what();
    return {};
  }
  if (!hasOnlyTrailingWhitespace(dataParse.datavalue(), parsedSize) || !std::isfinite(value)) {
    error = "BCD编码值非法: name=" + dataParse.name() + ", value=" + dataParse.datavalue();
    return {};
  }

  const double scaledDouble = value / factor;
  if (!std::isfinite(scaledDouble) || scaledDouble < 0.0 || scaledDouble > static_cast<double>(std::numeric_limits<std::int64_t>::max())) {
    error = "BCD编码缩放后值非法: name=" + dataParse.name() + ", value=" + dataParse.datavalue();
    return {};
  }
  const auto scaled = static_cast<std::int64_t>(std::llround(scaledDouble));
  const std::string scaledText = std::to_string(scaled);
  const std::size_t capacityDigits = dataSize * 2;
  if (scaledText.size() > capacityDigits) {
    error = "BCD编码超出点表长度: name=" + dataParse.name() + ", value=" + dataParse.datavalue() + ", scaled=" + scaledText + ", digits=" + std::to_string(scaledText.size()) + ", capacity=" + std::to_string(capacityDigits);
    return {};
  }

  std::string bcdDigits(capacityDigits - scaledText.size(), '0');
  bcdDigits += scaledText;
  std::deque<std::uint8_t> dataBytes;
  for (std::size_t i = 0; i < bcdDigits.size(); i += 2) {
    const auto high = static_cast<std::uint8_t>(bcdDigits[i] - '0');
    const auto low = static_cast<std::uint8_t>(bcdDigits[i + 1] - '0');
    dataBytes.emplace_front(static_cast<std::uint8_t>(((high << 4) | low) + 0x33));
  }
  return {dataBytes.begin(), dataBytes.end()};
}

std::mutex &dataSheetCacheMutex() {
  static std::mutex mutex;
  return mutex;
}

std::unordered_map<std::string, std::shared_ptr<const DataSheetCache>> &dataSheetCacheMap() {
  static std::unordered_map<std::string, std::shared_ptr<const DataSheetCache>> cacheMap;
  return cacheMap;
}

}  // namespace

struct DataSheetCache {
  std::string jsonContext;
  Dlt645Proto::DeviceData deviceTemplate;
  std::unordered_map<std::string, std::size_t> blockPointToIndex;
  std::unordered_map<std::string, std::string> modelToBlockName;
  std::string parseError;
};

DLT645::DLT645(std::string address, std::string dataSheetPath) {
  diSize_ = DI_SIZE;
  address_ = address;
  getDataSheet(dataSheetPath);
}
DLT645::~DLT645() {}
Dlt645Proto::DeviceData DLT645::getDeviceData() {
  Dlt645Proto::DeviceData result;
  if (!dataSheetCache_) {
    return result;
  }
  if (!dataSheetCache_->parseError.empty()) {
    SIREN_LOG_ERROR << "dataSheet解析错误: " << dataSheetCache_->parseError;
    result.set_deviceaddress(address_);
    return result;
  }
  result = dataSheetCache_->deviceTemplate;
  result.set_deviceaddress(address_);
  return result;
}
const std::string &DLT645::lastDecodeError() const {
  return lastDecodeError_;
}
Dlt645Proto::DeviceData DLT645::decodeRecvReadMeter(std::vector<std::uint8_t> rowData) {
  auto fail = [this](std::string reason) {
    lastDecodeError_ = std::move(reason);
    return Dlt645Proto::DeviceData();
  };

  lastDecodeError_.clear();
  Dlt645Proto::DeviceData message;
  if (rowData.empty()) {
    return fail("空报文");
  }
  if (rowData.size() < 12) {
    return fail("报文长度不足12字节");
  }
  if (rowData.front() != FRAME_HEAD) {
    return fail("首字节不是0x68");
  }
  if (rowData.size() <= SECOND_START_FLAG) {
    return fail("缺少第二个起始符");
  }
  if (rowData.at(SECOND_START_FLAG) != FRAME_HEAD) {
    return fail("第二个起始符不是0x68");
  }

  const std::size_t dataSize = rowData.at(DATA_SIZE_POS);
  const std::size_t frameSize = 12 + dataSize;
  if (rowData.size() < frameSize) {
    return fail("报文长度小于长度字段声明值");
  }
  if (rowData[frameSize - 1] != FRAME_END) {
    return fail("结束符不是0x16");
  }
  std::uint8_t cs = 0;
  for (std::size_t i = 0; i < 10 + dataSize; ++i) {
    cs += rowData[i];
  }
  if (cs != rowData[10 + dataSize]) {
    return fail("校验和错误");
  }
  if (dataSize < diSize_) {
    return fail("数据域长度小于DI长度");
  }

  const std::size_t payloadBegin = 10;
  const std::size_t payloadEnd = payloadBegin + dataSize;
  if (payloadEnd > rowData.size() - 2) {
    return fail("数据域越界");
  }

  std::deque<std::uint8_t> dataIdentDeq = std::deque<std::uint8_t>(rowData.begin() + payloadBegin, rowData.begin() + payloadBegin + diSize_);

  std::vector<std::uint8_t> rowDataItem = std::vector<std::uint8_t>(rowData.begin() + payloadBegin + diSize_, rowData.begin() + payloadEnd);
  for (auto &data : rowDataItem) {
    data -= 0x33;
  }
  std::string dataIdentStr;
  for (auto it = dataIdentDeq.rbegin(); it != dataIdentDeq.rend(); ++it) {
    std::ostringstream t_oss;
    t_oss << std::hex << std::setw(2) << std::setfill('0') << ((*it - 0x33) & 0xff);
    dataIdentStr += t_oss.str();
  }
  const auto dataIdentUpper = toUpperAscii(dataIdentStr);

  if (!dataSheetCache_) {
    return fail("datasheet缓存未初始化");
  }
  if (!dataSheetCache_->parseError.empty()) {
    lastDecodeError_ = "datasheet解析错误: " + dataSheetCache_->parseError;
    throw std::runtime_error("转换失败: " + dataSheetCache_->parseError);
  }

  const auto blockIndexIt = dataSheetCache_->blockPointToIndex.find(dataIdentUpper);
  if (blockIndexIt == dataSheetCache_->blockPointToIndex.end()) {
    return fail("未找到blockPoint=" + dataIdentUpper + "的点表定义");
  }

  message.set_devicename(dataSheetCache_->deviceTemplate.devicename());
  message.set_deviceaddress(dataSheetCache_->deviceTemplate.deviceaddress());
  auto *data = message.add_data();
  data->CopyFrom(dataSheetCache_->deviceTemplate.data(static_cast<int>(blockIndexIt->second)));

  std::size_t begin = 0;
  std::size_t end = 0;
  for (auto &dataParse : *data->mutable_dataparse()) {
    bool neg{false};
    begin = end;
    const auto dataParseSize = dataParse.datasize();
    if (dataParseSize <= 0) {
      return fail("点表dataSize非法: name=" + dataParse.name());
    }
    const auto chunkSize = static_cast<std::size_t>(dataParseSize);
    if (begin > rowDataItem.size() || chunkSize > rowDataItem.size() - begin) {
      return fail("数据长度不足: name=" + dataParse.name() + ", need=" + std::to_string(chunkSize) + ", remain=" + std::to_string(begin > rowDataItem.size() ? 0 : rowDataItem.size() - begin));
    }
    end = begin + chunkSize;
    std::deque<std::uint8_t> dataDeq = std::deque<std::uint8_t>(rowDataItem.begin() + begin, rowDataItem.begin() + end);
    if (dataDeq.empty()) {
      return fail("切片后数据为空: name=" + dataParse.name());
    }
    if (dataParse.encodetype() == "BCD" && dataParse.bcdmsbsign() && (*(dataDeq.end() - 1) & 0x80)) {
      auto lastData = *(dataDeq.end() - 1);
      dataDeq.pop_back();
      dataDeq.push_back(lastData & 0x7f);
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
      dataValueStr = std::to_string(std::stod(realData) * dataParse.factor());
      if (neg) {
        dataValueStr = "-" + dataValueStr;
      }
      if (dataParse.datatype() == "int") {
        dataValueStr = std::to_string(std::stoi(dataValueStr));
      } else if (dataParse.datatype() == "string") {
        // 应对BCD编码但是数据类型为string的情况
        // 针对不以ASCII进行编码的string
        dataValueStr = std::to_string(std::stoll(dataValueStr));
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
      for (int i = 0; i < static_cast<int>(realData.size()); i += 2) {
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
  return message;
}
std::string DLT645::resolveBlockName(const std::string &model) const {
  if (!dataSheetCache_) {
    return model;
  }
  const auto it = dataSheetCache_->modelToBlockName.find(model);
  if (it == dataSheetCache_->modelToBlockName.end()) {
    return model;
  }
  return it->second;
}
void DLT645::getDataSheet(std::string dataSheetPath) {
  dataSheetPath_ = std::move(dataSheetPath);
  dataSheetCache_ = loadDataSheetCache(dataSheetPath_);
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
      std::string encodeError;
      const auto dataBytes = encodeScaledBcdDataBytes(*dataParse, encodeError);
      if (!encodeError.empty()) {
        lastDecodeError_ = encodeError;
        SIREN_LOG_ERROR << encodeError;
        return {};
      }
      result.insert(result.end(), dataBytes.begin(), dataBytes.end());
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
    auto dataParse = data.dataparse().begin();
    std::string encodeError;
    const auto dataBytes = encodeScaledBcdDataBytes(*dataParse, encodeError);
    if (!encodeError.empty()) {
      lastDecodeError_ = encodeError;
      SIREN_LOG_ERROR << encodeError;
      return {};
    }
    result.insert(result.end(), dataBytes.begin(), dataBytes.end());
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

void DLT645::clearDataSheetCacheForTest() {
  std::lock_guard<std::mutex> lock(dataSheetCacheMutex());
  dataSheetCacheMap().clear();
}

std::size_t DLT645::getDataSheetCacheSizeForTest() {
  std::lock_guard<std::mutex> lock(dataSheetCacheMutex());
  return dataSheetCacheMap().size();
}

namespace {

std::shared_ptr<const DataSheetCache> buildDataSheetCache(const std::string &dataSheetPath) {
  std::ifstream dataSheetFile(dataSheetPath);
  if (!dataSheetFile.is_open()) {
    throw std::runtime_error("打开数据解析列表失败");
  }

  auto cache = std::make_shared<DataSheetCache>();
  cache->jsonContext = std::string((std::istreambuf_iterator<char>(dataSheetFile)), std::istreambuf_iterator<char>());
  dataSheetFile.close();

  google::protobuf::util::JsonParseOptions option;
  option.ignore_unknown_fields = true;
  const auto status = google::protobuf::util::JsonStringToMessage(cache->jsonContext, &cache->deviceTemplate, option);
  if (status != absl::OkStatus()) {
    cache->parseError = status.ToString();
    return cache;
  }

  for (int i = 0; i < cache->deviceTemplate.data_size(); ++i) {
    const auto &data = cache->deviceTemplate.data(i);
    const auto blockPointUpper = toUpperAscii(data.blockpoint());
    cache->blockPointToIndex.try_emplace(blockPointUpper, static_cast<std::size_t>(i));

    if (!data.blockname().empty()) {
      for (const auto &dataParse : data.dataparse()) {
        if (!dataParse.name().empty()) {
          cache->modelToBlockName.try_emplace(dataParse.name(), data.blockname());
        }
        for (const auto &bin : dataParse.binlist()) {
          if (!bin.binname().empty()) {
            cache->modelToBlockName.try_emplace(bin.binname(), data.blockname());
          }
        }
      }
    }
  }
  return cache;
}

std::shared_ptr<const DataSheetCache> loadDataSheetCache(const std::string &dataSheetPath) {
  std::lock_guard<std::mutex> lock(dataSheetCacheMutex());
  auto &cacheMap = dataSheetCacheMap();
  const auto cacheIt = cacheMap.find(dataSheetPath);
  if (cacheIt != cacheMap.end()) {
    return cacheIt->second;
  }

  auto cache = buildDataSheetCache(dataSheetPath);
  cacheMap.emplace(dataSheetPath, cache);
  return cache;
}

}  // namespace
}  // namespace dlt645
}  // namespace siren
