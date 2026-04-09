#ifndef MODBUS_TCP_H
#define MODBUS_TCP_H
#include <winsock2.h>
#include <vector>
#include <string>
#include <cstdint>
enum class Color : char {
    RED,
    GREEN,
    YELLOW,
    BUZZER
};
enum class OnOff : char{
    on,
    off
};

class AbstractRelay{
public:
    AbstractRelay()=default;
    virtual ~AbstractRelay()=default;
    //控制
    virtual void LightControl(Color color,OnOff onoff)=0;
    virtual bool isconnected()=0;
};

class RelayControl : public AbstractRelay
{
public:
    RelayControl(const std::string& ip, uint16_t port);
    ~RelayControl();
    //核心控制
    void LightControl(Color color,OnOff onoff);

    bool isconnected(){
        return connected;
    }
private:
    // 写单个线圈
    bool writeSingleCoil(uint8_t slaveId, uint16_t coilAddr, bool on);
    // 写多个线圈 0x0F
    bool writeMultipleCoils(uint8_t slaveId, uint16_t startAddr, const std::vector<bool>& states);
    // 读线圈
    std::vector<bool> readCoils(uint8_t slaveId, uint16_t startAddr, uint16_t qty);
    // 翻转线圈 0x05 特殊值 0x5500
    bool toggleSingleCoil(uint8_t slaveId, uint16_t coilAddr);


private:
    SOCKET sock_;
    // RTU 帧收发
    void sendRecv(const std::vector<uint8_t>& req, std::vector<uint8_t>& resp, size_t respLen);
    // 生成 CRC
    uint16_t calcCRC(const uint8_t* data, size_t len);
    // 构造 RTU 帧
    std::vector<uint8_t> buildFrame(uint8_t slaveId, uint8_t func, const std::vector<uint8_t>& data);

    bool connected;
};
#endif // MODBUS_TCP_H
