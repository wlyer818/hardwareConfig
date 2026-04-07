#pragma once
#include <windows.h>
#include <string>
#include <vector>

class SerialPort
{
public:
    SerialPort();
    ~SerialPort();

    bool open(const std::string& portName, DWORD baudRate = 9600,
              BYTE parity = NOPARITY, BYTE byteSize = 8, BYTE stopBits = TWOSTOPBITS);
    void close();

    bool write(const std::string& data);
    bool writeBytes(const unsigned char* data, size_t len);
    std::string read(size_t expectedLen = 0, DWORD timeout = 200);

    bool isOpen() const { return m_handle != INVALID_HANDLE_VALUE; }
    HANDLE getHandle() const { return m_handle; }  
private: 
    HANDLE m_handle;
};
