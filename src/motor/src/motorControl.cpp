#include "MotorControl.h"
#include <iostream>
#include <thread>
#include <chrono>

/* ======================================================
 * CRC16(MODBUS)
 * ====================================================== */
unsigned short MotorControl::crc16(const unsigned char* data, unsigned int length)
{
    unsigned short crc = 0xFFFF;
    for (unsigned int i = 0; i < length; i++) {
        crc ^= (unsigned short)data[i];
        for (unsigned int j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/* ------------------------------------------------------ */
MotorControl::MotorControl(
    const std::string& com,DWORD baud) : m_com(com),m_baud(baud) {}
MotorControl::~MotorControl() { close(); }

/* ------------------------------------------------------ */
bool MotorControl::open()
{
    return sp.open(m_com, m_baud, NOPARITY, 8, TWOSTOPBITS);
}

void MotorControl::close()
{
    sp.close();
}

/* ======================================================
 *  基础帧构造
 * ====================================================== */
std::string MotorControl::buildWrite(uint16_t reg, int16_t value)
{
    unsigned char buf[8];
    buf[0] = devID;
    buf[1] = 0x06;
    buf[2] = reg >> 8;
    buf[3] = reg & 0xFF;
    buf[4] = (value >> 8) & 0xFF;
    buf[5] = value & 0xFF;
    unsigned short crc = crc16(buf, 6);
    buf[6] = crc & 0xFF;
    buf[7] = crc >> 8;
    return std::string(reinterpret_cast<char*>(buf), 8);
}

std::string MotorControl::buildRead(uint16_t reg, uint16_t count)
{
    unsigned char buf[8];
    buf[0] = devID;
    buf[1] = 0x03;
    buf[2] = reg >> 8;
    buf[3] = reg & 0xFF;
    buf[4] = count >> 8;
    buf[5] = count & 0xFF;
    unsigned short crc = crc16(buf, 6);
    buf[6] = crc & 0xFF;
    buf[7] = crc >> 8;
    return std::string(reinterpret_cast<char*>(buf), 8);
}

/* ======================================================
 * CRC 校验
 * ====================================================== */
bool MotorControl::verifyCRC(const std::string& frame)
{
    if (frame.size() < 3) return false;
    unsigned short calc = crc16(
        (unsigned char*)frame.data(),
        frame.size() - 2
    );
    const unsigned char* p = (const unsigned char*)frame.data();
    return (p[frame.size() - 2] == (calc & 0xFF) &&
            p[frame.size() - 1] == ((calc >> 8) & 0xFF));
}

/* ======================================================
 * 基础写入 + 回显校验
 * ====================================================== */
bool MotorControl::writeAndEcho(const std::string& frame, DWORD timeout)
{
    if (!sp.writeBytes((unsigned char*)frame.data(), frame.size()))
        return false;

    std::string echo = sp.read(8, timeout);
    if (echo.size() != 8 || !verifyCRC(echo)) {
        std::cerr << "[ERR] Echo invalid\n";
        return false;
    }

    if (echo.compare(0, 6, frame, 0, 6) != 0) {
        std::cerr << "[ERR] Echo mismatch\n";
        return false;
    }

    return true;
}

bool MotorControl::writeMaybeEcho(const std::string& frame, bool waitEcho, DWORD timeout)
{
    if (!sp.writeBytes((unsigned char*)frame.data(), frame.size()))
        return false;

    if (!waitEcho) return true;

    std::string echo = sp.read(16, timeout);
    if (echo.size() < 8) return true;

    std::string head8 = echo.substr(0, 8);
    if (!verifyCRC(head8)) return true;

    return true;
}

/* ======================================================
 * 读一个寄存器
 * ====================================================== */
bool MotorControl::readU16(uint16_t reg, uint16_t& out, DWORD timeout)
{
    std::string req = buildRead(reg, 1);
    sp.writeBytes((unsigned char*)req.data(), req.size());

    std::string resp = sp.read(7, timeout);
    if (resp.size() != 7 || !verifyCRC(resp)) {
        std::cerr << "[ERR] Read invalid\n";
        return false;
    }

    const unsigned char* d = (const unsigned char*)resp.data();
    out = (d[3] << 8) | d[4];
    return true;
}

/* ======================================================
 * 业务封装 设置加速/减速、运动参数、启动运动等
 * ====================================================== */
bool MotorControl::setAccelDecel(int accel_ms, int decel_ms)
{
    return writeAndEcho(buildWrite(0x0028, accel_ms)) &&
           writeAndEcho(buildWrite(0x0029, decel_ms));
}

bool MotorControl::setMotionParams(int speed, int turns, int pulses)
{
    return writeAndEcho(buildWrite(0x0204, speed))  &&   // rpm
           writeAndEcho(buildWrite(0x0202, turns))  &&   // turns (+/-)
           writeAndEcho(buildWrite(0x0203, pulses));     // pulses
}
/*=======================================================
 * 启动运动
 * ====================================================== */
bool MotorControl::triggerStart()
{
    writeAndEcho(buildWrite(0x011F, 0));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return writeAndEcho(buildWrite(0x011F, 1));
}
/*=====================================================
 *读取读取报警状态, 运行状态, 位置误差
 *===================================================== */
bool MotorControl::readAlarm(uint16_t& alarm)
{
    return readU16(0x1013, alarm);
}

bool MotorControl::readStatus(uint16_t& status)
{
    return readU16(0x1016, status);
}

bool MotorControl::readPosError(int32_t& error)
{
    uint16_t lo = 0, hi = 0;

    if (!readU16(0x1005, lo)) return false;
    if (!readU16(0x1006, hi)) return false;

    uint32_t raw = ((uint32_t)hi << 16) | lo;

    error = static_cast<int32_t>(raw);  // ⭐ 关键转换
    return true;
}
bool MotorControl::readSpeed(int16_t& speed)
{
    uint16_t raw = 0;
    if (!readU16(0x1000, raw)) return false;

    speed = static_cast<int16_t>(raw);  // ⭐ 关键转换
    return true;
}
/*
* speed:运行速度
* turns:圈数（整数部分）
* pluses:圈数（小数部分）0~10000
*/
bool MotorControl::run(int speed, int turns, int pulses)
{
    if (!setAccelDecel(200, 200)) return false;
    if (!setMotionParams(speed, turns, pulses)) return false;
    if (!triggerStart()) return false;

    return true;
}
/*======================================================
 * 等待停止或位置稳定 TODO:需确定义停止状态和位置误差阈值
 * ====================================================== */ 
bool MotorControl::waitUntilStop(int timeout_ms)
{
    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        uint16_t status = 0;
        if (!readStatus(status))
            return false;

        // 示例：bit0=1表示运行中，=0表示停止
        bool running = status & 0x0001;

        if (!running)
            return true;

        auto now = std::chrono::steady_clock::now();
        int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if (elapsed > timeout_ms)
        {
            std::cerr << "[ERR] Motor stop timeout\n";
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
bool MotorControl::waitUntilPosStable(uint32_t threshold, int timeout_ms)
{
    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        int32_t err = 0;
        readPosError(err);

        if (err < threshold)
            return true;

        auto now = std::chrono::steady_clock::now();
        int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if (elapsed > timeout_ms)
        {
            std::cerr << "[ERR] Motor pos not stable\n";
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
