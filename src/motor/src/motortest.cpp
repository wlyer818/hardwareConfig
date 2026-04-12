#include "MotorControl.h"
#include <iostream>
#include <thread>

int main()
{
    MotorControl motor("COM4",9600);
    // 读状态
    uint16_t  status = 0;
    int16_t speed = 0;
    int32_t posErr = 0;

    if (!motor.open()) {
        std::cout << "Failed to open motor control\n";
        return 0;
    }
    // 执行动作
    // motor.run(80, -50, 0);// 反转 5 转往右走
    motor.run(80, 50, 0);// 正转 5 转往左走
    Sleep(2000);
    while(true){
        motor.readStatus(status);// 读取运行状态
        motor.readPosError(posErr);// 读取位置误差
        motor.readSpeed(speed);// 读取速度
        std::cout<< "  Status=" << status
                << "  speed=" << speed
                << "  PosErr=" << posErr << std::endl;
        if(speed == 0) 
            break;
        else
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    motor.close();
    return 0;
}
