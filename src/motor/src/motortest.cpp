#include "MotorControl.h"
#include <iostream>
#include <thread>

int main()
{
    MotorControl motor("COM4",9600);

    if (!motor.open()) {
        std::cout << "Failed to open motor control\n";
        return 0;
    }

    // 执行动作
    // motor.run(60, -50, 0);// 反转 5 转往右走
    motor.run(60, 50, 0);// 正转 5 转往左走

    // 读状态
    uint16_t alarm = 0, status = 0;
    int16_t speed = 0;
    int32_t posErr = 0;
    for(int i = 1;i<=40;i++){
            
        motor.readAlarm(alarm);// 读取报警状态
        motor.readStatus(status);// 读取运行状态
        motor.readPosError(posErr);// 读取位置误差
        motor.readSpeed(speed);// 读取位置误差
        std::cout<<"di"<<i<<"miao:\n";
        std::cout << "Alarm=" << alarm
                << "  Status=" << status
                << "  speed=" << speed
                << "  PosErr=" << posErr << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    motor.close();
    return 0;
}
