#pragma once
#include "serial.h"
#include <string>
#include <cstdint>

class AbstractMotorCtl{
public:
    AbstractMotorCtl()=default;
    virtual ~AbstractMotorCtl()=default;

    virtual bool open()=0;
    virtual void close()=0;

    virtual bool run(int speed, int turns, int pulses)=0;
};

class MotorControl : public AbstractMotorCtl
{
public:
    MotorControl(const std::string& com,DWORD baud);
    ~MotorControl();

    bool open();
    void close();
private:
    /* --- 高层业务流程 --- */
    //设置加速减速时间
    bool setAccelDecel(int accel_ms, int decel_ms);
    bool setMotionParams(int speed_rpm, int turns, int pulses);
    bool triggerStart();              // 沿触发：0→1
public:
    //读取报警状态
    bool readAlarm(uint16_t& alarm);
    //读取运行状态
    bool readStatus(uint16_t& status);
    //读取位置误差
    bool readPosError(int32_t& error);
    bool readSpeed(int16_t& speed);

    bool run(int speed, int turns, int pulses);
    bool waitUntilStop(int timeout_ms=10000);
    bool waitUntilPosStable(uint32_t threshold, int timeout_ms=10000);

private:
    SerialPort sp;            //串口通信控制
    uint8_t devID = 1;        //默认从站 ID=1
    std::string m_com;        //使用串口号
    DWORD m_baud;

private:
    /* --- 基础 Modbus 工具函数 --- */
    std::string buildWrite(uint16_t reg, int16_t value);
    std::string buildRead(uint16_t reg, uint16_t count);
    bool verifyCRC(const std::string& frame);
    bool writeAndEcho(const std::string& frame, DWORD timeout = 300);
    bool writeMaybeEcho(const std::string& frame, bool waitEcho = true, DWORD timeout = 300);
    bool readU16(uint16_t reg, uint16_t& out, DWORD timeout = 300);

    unsigned short crc16(const unsigned char* data, unsigned int len);
};
