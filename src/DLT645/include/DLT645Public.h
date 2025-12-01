#pragma once

#include <cstdint>

namespace siren {
const std::uint8_t FRAME_HEAD = 0x68;
const std::uint8_t FRAME_END = 0x16;
const std::size_t DI_SIZE = 4;
const std::size_t SECOND_START_FLAG = 7;
const std::size_t DATA_SIZE_POS = 9;

// enum class CtrlCode : std::uint8_t {
//     READ_DATA = 0x11,
//     READ_FOLLOW_DATA = 0x12,
//     WRITE_DATA = 0x14,
//     READ_COMM_ADDR = 0x13,
//     WRITE_COMM_ADDR = 0x15,
//     BROADCAST_TIME = 0x08,
//     FREEZE = 0x16,
//     CHANGE_COMM_RATE = 0x17,
//     CHANGE_PASSWORD = 0x18,
//     MAX_DEMAND_RESET = 0x19,
//     AMMETER_RESET = 0x1a,
//     EVENT_RESET = 0x1b,
//     SECURITY_AUTH = 0x03,
// };
enum class DI3 : std::uint8_t {
  ChangeDataIdent = 0x02,  // 变量数据标识
  Statistics = 0x03,       // 统计数据
  Param = 0x04,            // 参数
  Ctrl = 0x06,             // 控制
};
enum class DI2 : std::uint8_t {
  /*** ChangeDataIdent 变量数据编码表 ****************************************/
  Voltage = 0x01,        // 电压，DI1: A相 0x01, B相 0x02, C相 0x03 电压数据块 0xff // DI0:0
  Current = 0x02,        // 电流, DI1: A相 0x01, B相 0x02, C相 0x03 电流数据块 0xff // DI0:0
  ActivePower = 0x03,    // 瞬时有功功率, DI1: 瞬时总有功功率 0x00, 瞬时A有功功率 0x01, 瞬时B有功功率 0x02,
                         // 瞬时C有功功率 0x03, 瞬时有功功率数据块 0xff // DI0:0
  ReactivePower = 0x04,  // 瞬时无功功率, DI1: 瞬时总无功功率 0x00, 瞬时A无功功率 0x01, 瞬时B无功功率 0x02,
                         // 瞬时C无功功率 0x03, 瞬时无功功率数据块 0xff // DI0:0
  ApparentPower = 0x05,  // 瞬时视在功率, DI1: 瞬时总视在功率 0x00, 瞬时A视在功率 0x01, 瞬时B视在功率 0x02,
                         // 瞬时C视在功率 0x03, 瞬时视在功率数据块 0xff // DI0:0
  Factor = 0x06,         // 功率因数, DI1: 总功率因数 0x00, A功率因数 0x01, B功率因数 0x02, C功率因数 0x03 // DI0:0
  Hz = 0x80,             // DI1:0 // 电网频率, DI0:0x02
  /**************************************************************************/
  /*** Statistics ***********************************************************/
  Statistics = 0x81,  // 日发电量, DI1: 0x05, DI0: 0x01;
                      // 年发电量, DI1: 0x05, DI0: 0x03;
  /**************************************************************************/
  /*** Param ****************************************************************/
  Device = 0x00,  // 设备类型, DI1: 0x00, DI0: 0x02;
                  // 额定功率, DI1: 0x01, DI0: 0x0f;
                  // 设备ID, DI1: 0x00, DI0: 0x05;
  /**************************************************************************/
  /*** Ctrl *****************************************************************/
  SE_Switch = 0x01  // 逆变器开关机控制, DI1: 0x07, DI0: 0x01;
                    /**************************************************************************/
};
}  // namespace siren