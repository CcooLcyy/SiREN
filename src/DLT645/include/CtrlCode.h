#pragma once

#include <cstdint>

namespace siren {
namespace CtrlCode {
enum class FuncCode : std::uint8_t {
  RETAIN = 0x00,            // 保留
  BROADCAST_TIME = 0x08,    // 广播校时
  READ_DATA = 0x11,         // 读数据
  READ_SUB_DATA = 0x12,     // 读后续数据
  READ_COMM_ADDR = 0x13,    // 读通信地址
  WRITE_DATA = 0x14,        // 写数据
  BROADCAST_FREEZE = 0x16,  // 广播冻结
  MODIFY_PASSWORD = 0x18,   // 修改密码
  DATA_CLEAR = 0x1A,        // 数据清零
  EVENT_CLEAR = 0x1B,       // 事件清零
  CTRL_FUNC = 0x1c,         // 控制指令
  OTA = 0x1d,               // 在线升级
  DATA_FORWARD = 0x1e,      // 数据转发
};
}  // namespace CtrlCode
}  // namespace siren