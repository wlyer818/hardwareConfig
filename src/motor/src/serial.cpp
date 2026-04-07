#include "serial.h"
#include <iostream>

SerialPort::SerialPort() : m_handle(INVALID_HANDLE_VALUE) {}
SerialPort::~SerialPort() { close(); }

bool SerialPort::open(const std::string& portName, DWORD baudRate, BYTE parity, BYTE byteSize, BYTE stopBits)
{
    close();
    std::string fullName = "\\\\.\\" + portName;
    m_handle = CreateFileA(fullName.c_str(),
                           GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           OPEN_EXISTING, 0, nullptr);
    if (m_handle == INVALID_HANDLE_VALUE) 
      return false;

    SetupComm(m_handle, 1024, 1024);

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(m_handle, &dcb))
        return false;

    dcb.BaudRate = baudRate;
    dcb.Parity = parity;
    dcb.ByteSize = byteSize;
    dcb.StopBits = stopBits;

    if (!SetCommState(m_handle, &dcb))
        return false;

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 300;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 300;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(m_handle, &timeouts);

    PurgeComm(m_handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return true;
}

void SerialPort::close()
{
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

bool SerialPort::write(const std::string& data)
{
    return writeBytes(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

bool SerialPort::writeBytes(const unsigned char* data, size_t len)
{
    if (m_handle == INVALID_HANDLE_VALUE)
        return false;

    DWORD written = 0;
    if (!WriteFile(m_handle, data, (DWORD)len, &written, nullptr))
        return false;
    return written == len;
}

std::string SerialPort::read(size_t expectedLen, DWORD timeout)
{
    if (m_handle == INVALID_HANDLE_VALUE)
        return "";

    std::string result;
    DWORD startTick = GetTickCount();

    while (true)
    {
        DWORD bytesRead = 0;
        char buffer[256];
        if (ReadFile(m_handle, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0)
            result.append(buffer, bytesRead);

        if (expectedLen > 0 && result.size() >= expectedLen)
            break;

        if (GetTickCount() - startTick > timeout)
            break;

        Sleep(10);
    }
    return result;
}
