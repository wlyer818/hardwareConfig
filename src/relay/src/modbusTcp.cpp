#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // 0x0600 代表 Windows Vista，这是支持 inet_pton 的最低版本
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include "modbusTcp.h"
#include <cstring>
#include <iostream>
#include <vector>
#include <cstdint>
#pragma comment(lib, "ws2_32.lib") // 确保链接库
RelayControl::RelayControl(const std::string& ip, uint16_t port)
    : sock_(INVALID_SOCKET), connected(false)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ == INVALID_SOCKET)
    {
        WSACleanup();
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (InetPtonA(AF_INET, ip.c_str(), &addr.sin_addr) != 1)
    {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        WSACleanup();
        return;
    }

    if (connect(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        WSACleanup();
        return;
    }

    connected = true;
}


RelayControl::~RelayControl() {
    if (sock_ != INVALID_SOCKET) closesocket(sock_);
    WSACleanup();
}
//还得完善，按照红绿蓝三灯对应硬件接口来写
void RelayControl::LightControl(Color color, OnOff onoff)
{
    switch (color) {
    case Color::RED:
        if(onoff==OnOff::on){
            writeSingleCoil(0,0x0000,true);
        }
        else{
            writeSingleCoil(0,0x0000,false);
        }
        break;
    case Color::BLUE:
        if(onoff==OnOff::on){
            writeSingleCoil(0,0x0001,true);
        }
        else{ 
            writeSingleCoil(0,0x0001,false);
        }
        break;
    case Color::GREEN:
        if(onoff==OnOff::on){
            writeSingleCoil(0,0x0002,true);
        }
        else{
            writeSingleCoil(0,0x0002,false);
        }
        break;
    case Color::BUZZER:
        if(onoff ==OnOff::on){
            writeSingleCoil(0,0x0003,true);
        }
        else{
            writeSingleCoil(0,0x0003,false);
        }
    default:
        break;
    }
}

// CRC16(Modbus)
uint16_t RelayControl::calcCRC(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for(size_t i=0; i<len; i++) {
        crc ^= data[i];
        for(int j=0;j<8;j++) {
            if(crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>=1;
        }
    }
    return crc;
}

// 构造 RTU 帧
std::vector<uint8_t> RelayControl::buildFrame(uint8_t slaveId, uint8_t func, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> frame;
    frame.emplace_back(slaveId); 
    frame.emplace_back(func);
    frame.insert(frame.end(), data.begin(), data.end());

    uint16_t crc = calcCRC(frame.data(), frame.size());
    frame.push_back(uint8_t(crc & 0xFF));       // CRC低字节
    frame.push_back(uint8_t(crc >> 8));         // CRC高字节

    return frame;
}

// 发送 RTU 帧并接收固定长度响应
void RelayControl::sendRecv(const std::vector<uint8_t>& req, std::vector<uint8_t>& resp, size_t respLen) {
    if (send(sock_, (char*)req.data(), (int)req.size(), 0) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        std::cerr << "send failed, WSA error = " << err << std::endl;
        return;
    }


    resp.resize(respLen);
    int n = recv(sock_, (char*)resp.data(), (int)respLen, 0);
    if (n <= 0) {
        int err = WSAGetLastError();
        std::cerr << "recv failed, WSA error = " << err << std::endl;
        return;
    }

    resp.resize(n);
}

// 写单个线圈 0x05
bool RelayControl::writeSingleCoil(uint8_t slaveId, uint16_t coilAddr, bool on) {
    std::vector<uint8_t> data = {
        uint8_t(coilAddr >> 8), uint8_t(coilAddr & 0xFF),
        uint8_t(on ? 0xFF : 0x00), 0x00
    };
    auto req = buildFrame(slaveId, 0x05, data);

    std::vector<uint8_t> resp;
    sendRecv(req, resp, 8); // 8字节固定响应

    return resp.size() == 8 && resp[0]==slaveId && resp[1]==0x05;
}
// 写多个线圈 0x0F
bool RelayControl::writeMultipleCoils(uint8_t slaveId, uint16_t startAddr, const std::vector<bool>& states) {
    uint16_t qty = uint16_t(states.size());
    uint8_t byteCount = uint8_t((qty + 7)/8);
    std::vector<uint8_t> data(byteCount + 5); // 起始地址(2)+数量(2)+字节数(1)+数据
    data[0] = uint8_t(startAddr >> 8);
    data[1] = uint8_t(startAddr & 0xFF);
    data[2] = uint8_t(qty >> 8);
    data[3] = uint8_t(qty & 0xFF);
    data[4] = byteCount;

    for(int i=0;i<qty;i++)
        if(states[i]) data[5 + i/8] |= (1 << (i%8));

    auto req = buildFrame(slaveId, 0x0F, data);
    std::vector<uint8_t> resp;
    sendRecv(req, resp, 8); // 回显起始地址+数量

    return resp.size() == 8 && resp[0]==slaveId && resp[1]==0x0F;
}
// 读线圈 0x01
std::vector<bool> RelayControl::readCoils(uint8_t slaveId, uint16_t startAddr, uint16_t qty) {
    std::vector<uint8_t> data = {
        uint8_t(startAddr >> 8), uint8_t(startAddr & 0xFF),
        uint8_t(qty >> 8), uint8_t(qty & 0xFF)
    };
    auto req = buildFrame(slaveId, 0x01, data);

    size_t byteCnt = (qty + 7)/8;
    std::vector<uint8_t> resp;
    sendRecv(req, resp, 5 + byteCnt); // Slave+Func+ByteCnt+Data+CRC(2)

    std::vector<bool> res(qty, false);
    if(resp.size() < 5) return res;
    for(int i=0;i<qty;i++) {
        res[i] = (resp[3 + i/8] >> (i%8)) & 1;
    }
    return res;
}
// 翻转线圈 0x05 特殊值 0x5500
bool RelayControl::toggleSingleCoil(uint8_t slaveId, uint16_t coilAddr) {
    std::vector<uint8_t> data = {
        uint8_t(coilAddr >> 8), uint8_t(coilAddr & 0xFF),
        0x55, 0x00
    };
    auto req = buildFrame(slaveId, 0x05, data);
    std::vector<uint8_t> resp;
    sendRecv(req, resp, 8);

    return resp.size()==8 && resp[0]==slaveId && resp[1]==0x05;
}
