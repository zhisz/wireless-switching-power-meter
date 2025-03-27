/*
 * @LastEditors: qingmeijiupiao
 * @Description: 功率表相关部分
 * @note: 该文件为普通版本和PRO版本的功率表的实现，通过宏 IS_PRO_VERSION 来控制 在pltformio.ini 中添加宏定义来控制是PRO版本还是普通版
 * @Author: qingmeijiupiao
 * @LastEditTime: 2025-03-27 16:19:22
 */
#ifndef POWERMETER_HPP
#define POWERMETER_HPP

#include "FixedSizeQueue.hpp"
#include "static/HXCthread.hpp"
#include "HXC_NVS.hpp"

#ifdef IS_PRO_VERSION
#pragma message("------------------PRO VERSION------------------")
//↓↓↓↓↓↓↓↓↓↓PRO版本↓↓↓↓↓↓↓↓↓↓
#include <Adafruit_INA228.h>//INA228 PRO版本
//↑↑↑↑↑↑↑↑↑↑PRO版本↑↑↑↑↑↑↑↑↑↑
#else
#pragma message("------------------NORMAL VERSION------------------")
//↓↓↓↓↓↓↓↓↓↓普通版↓↓↓↓↓↓↓↓↓↓
#include "INA226.h"//INA226 普通版本
//↑↑↑↑↑↑↑↑↑↑普通版↑↑↑↑↑↑↑↑↑↑
#endif


// 输入电压电流读取相关
namespace POWERMETER {
    #ifdef IS_PRO_VERSION // PRO版本
    constexpr float sample_resistance_default = 0.5;
    #else // 普通版
    constexpr float sample_resistance_default = 2.0;
    #endif

    HXC::NVS_DATA<float> sample_resistance("resistance",sample_resistance_default);// NVS数据，采样电阻的值，默认为2mΩ
    constexpr int READ_HZ = 100;                  // 读取电压电流的频率
    constexpr int data_save_time=6;              // 保存数据的时间 秒
    float voltage = 0;                       // 电压
    float current = 0;                       // 电流
    float output_mah = 0;                    // 输出的毫安时
    float output_mwh = 0;                    // 输出的毫瓦时
    float MAX_CURRENT = 0;                    // 最大电流
    float MAX_VOLTAGE = 0;                    // 最大电压
    uint32_t last_time = millis();             // 上次读取的时间
    FixedSizeQueue<float,READ_HZ*data_save_time> voltage_queue;         // 电压队列
    FixedSizeQueue<float,READ_HZ*data_save_time> current_queue;         // 电流队列
    FixedSizeQueue<float,READ_HZ*data_save_time> power_queue;         // 功率队列
    #ifdef IS_PRO_VERSION // PRO版本
    Adafruit_INA228 PowerSensor=Adafruit_INA228(); // 创建INA228传感器对象
    //↑↑↑↑↑↑↑↑↑↑PRO版本↑↑↑↑↑↑↑↑↑↑
    #else
    INA226 PowerSensor=INA226(0x40); // 创建INA226传感器对象
    //↑↑↑↑↑↑↑↑↑↑普通版↑↑↑↑↑↑↑↑↑↑
    #endif

    // 初始化
    void setup(){
        Wire.setPins(5, 4);                   // 设置I2C引脚
        Wire.begin();                         // 初始化I2C


        #ifdef IS_PRO_VERSION
        //↓↓↓↓↓↓↓↓↓↓PRO版本↓↓↓↓↓↓↓↓↓↓
        if (!PowerSensor.begin(0x40,&Wire)) {           // 检查INA228是否连接
            Serial.println("INA228 sensor not found!");
        }
        PowerSensor.setAveragingCount(INA228_COUNT_16); // 设置平均采样次数
        PowerSensor.setVoltageConversionTime(INA228_TIME_1052_us);// 设置转换时间
        PowerSensor.setCurrentConversionTime(INA228_TIME_1052_us); // 设置转换时间
        //↑↑↑↑↑↑↑↑↑↑PRO版本↑↑↑↑↑↑↑↑↑↑
        #else
        //↓↓↓↓↓↓↓↓↓↓普通版↓↓↓↓↓↓↓↓↓↓
        if (!PowerSensor.begin()) {           // 检查INA228是否连接
            Serial.println("INA226 sensor not found!");
        }
        PowerSensor.setAverage(INA226_16_SAMPLES); // 设置平均采样次数
        PowerSensor.setShuntVoltageConversionTime(INA226_8300_us);// 设置转换时间
        //↑↑↑↑↑↑↑↑↑↑普通版↑↑↑↑↑↑↑↑↑↑
        #endif
    };

    // 更新电压电流的任务
    HXC::thread<void> updatePower_thread([]() {
        // 初始化
        POWERMETER::setup();
        while (true) {
            #ifdef IS_PRO_VERSION // PRO版本
            //↓↓↓↓↓↓↓↓↓↓PRO版本↓↓↓↓↓↓↓↓↓↓
            voltage = PowerSensor.readBusVoltage()/1000.f; // 读取电压
            float row_data = PowerSensor.readShuntVoltage(); // 读取寄存器中的电流数据
            last_time = millis();              // 更新上次读取时间
            // 处理电流数据
            current = float(row_data)/sample_resistance;     // 转换电流数据
            //↑↑↑↑↑↑↑↑↑↑PRO版本↑↑↑↑↑↑↑↑↑↑
            #else
            //↓↓↓↓↓↓↓↓↓↓普通版↓↓↓↓↓↓↓↓↓↓
            voltage = PowerSensor.getBusVoltage(); // 读取电压
            int16_t row_data = PowerSensor.getRegister(1); // 读取寄存器中的电流数据
            last_time = millis();              // 更新上次读取时间
            // 处理电流数据
            current = float(row_data) * 0.0025/sample_resistance;     // 转换电流数据
            //↑↑↑↑↑↑↑↑↑↑普通版↑↑↑↑↑↑↑↑↑↑
            #endif
            // 更新最大电压和最大电流
            if (voltage > MAX_VOLTAGE) MAX_VOLTAGE = voltage;
            if (current > MAX_CURRENT) MAX_CURRENT = current;

            // 计算输出的毫安时和毫瓦时
            output_mah += current * (millis() - last_time) / 3600.0;
            output_mwh += voltage * current * (millis() - last_time) / 3600.0;

            // 更新电压队列
            voltage_queue.push(voltage);
            // 更新电流队列
            current_queue.push(current);
            // 更新功率队列
            power_queue.push(voltage * current);

            delay(1000 / READ_HZ);             // 延时，保持读取频率
        }
    });
}

#endif