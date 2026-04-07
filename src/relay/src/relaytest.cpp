#include "modbusTcp.h"
#include <iostream>

int main() {
    //连接Modbus TCP服务器
    RelayControl relayControlor("192.168.11.100", 8888);
    if(!relayControlor.isconnected()) {
        std::cerr << "Failed to connect to relay controller." << std::endl;
        return -1;
    }   
    //亮红灯
    relayControlor.LightControl(Color::RED, OnOff::on);
    Sleep(2000); //等待2秒
    //灭红灯
    relayControlor.LightControl(Color::RED, OnOff::off);
    //亮蓝灯
    relayControlor.LightControl(Color::BLUE, OnOff::on);
    Sleep(2000); //等待2秒
    //灭蓝灯
    relayControlor.LightControl(Color::BLUE, OnOff::off);
    //亮绿灯
    relayControlor.LightControl(Color::GREEN, OnOff::on);   
    Sleep(2000); //等待2秒
    //灭绿灯
    relayControlor.LightControl(Color::GREEN, OnOff::off);
    //响蜂鸣器
    relayControlor.LightControl(Color::BUZZER, OnOff::on);
    Sleep(2000); //等待2秒
    //灭蜂鸣器
    relayControlor.LightControl(Color::BUZZER, OnOff::off);
    return 0;
}
