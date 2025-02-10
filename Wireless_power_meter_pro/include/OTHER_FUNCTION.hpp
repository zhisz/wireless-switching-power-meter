/*
 * @LastEditors: qingmeijiupiao
 * @Description: 其他功能，如串口打印，过流保护，过放保护
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 17:16:26
 */

#ifndef OTHER_FUNCTION_HPP
#define OTHER_FUNCTION_HPP
#include <Arduino.h>
#include "static/HXC_NVS.hpp"
#include "static/POWERMETER.hpp"
#include "static/powerctrl.hpp"
#include "static/TemperatureSensor.hpp"// 温度传感器
#include "static/HXCthread.hpp"

extern POWERCTRL_t power_output;
namespace OTHER_FUNCTION{
    /*#############串口打印######################*/
    // 串口打印数据
    TaskHandle_t serial_print_handle = nullptr;
    HXC::thread<void> serial_print_thread([](){
        while (true){
            Serial.print("V&A&W:");// 串口打印数据
            // 输出电压:%f
            Serial.print(POWERMETER::voltage);
            Serial.print(",");
            // 输出电流:%f
            Serial.print(POWERMETER::current);
            Serial.print(",");
            // 输出功率:%f
            Serial.println(POWERMETER::voltage*POWERMETER::current);
            
            delay(1000/POWERMETER::READ_HZ);
        }
    });
    // 串口打印数据控制
    void serial_print_ctrl(bool state) {
        if(state){
            serial_print_thread.start(/*taskname=*/"serial_print",/*stacksize=*/2048);// 串口打印数据任务启动
        }else{
            serial_print_thread.stop();
        }
    }
    /*#############串口打印######################*/

    /*#############电流保护######################*/
    HXC::NVS_DATA<bool> default_current_protect_state(/*NVS key=*/"current_p_sta",/*default value=*/false);// 默认电流保护状态

    HXC::NVS_DATA<float> current_protect_value("current_value",40);// 电流保护值,A

    // 电流保护任务
    HXC::thread<void> current_protect_thread([](){
        while (true){   
            if(POWERMETER::current>current_protect_value){
                power_output.off();
                buzz.buzz(0.5);
                delay(5000);
            }
            delay(2);
        }
    });

    // 开关电流保护
    void current_protect_ctrl(bool state) {
        if(state){
            current_protect_thread.start(/*taskname=*/"current_protect",/*stacksize=*/2048);// 电流保护任务启动
        } else {
            current_protect_thread.stop();
        }
    }
    /*#############电流保护######################*/


    /*#############低电压保护######################*/
    HXC::NVS_DATA<bool> default_low_voltage_protect_state("voltage_p_sta",false);// 默认电流保护状态
    HXC::NVS_DATA<float> low_voltage_protect_value("low_voltage_value",12);// 电压保护值,单位V
    // 低电压保护任务
    HXC::thread<void> voltage_protect_thread([](){
        while (true){
            if(POWERMETER::voltage<low_voltage_protect_value){
                power_output.off();
                buzz.buzz(0.5);
                delay(5000);
            }
            delay(10);
        }
    });
    
    // 开关低压保护
    void voltage_protect_ctrl(bool state) {
        if(state){
            voltage_protect_thread.start(/*taskname=*/"voltage_protect",/*stacksize=*/2048);// 低电压保护任务启动
        } else {
            voltage_protect_thread.stop();
        }
    }
    /*#############低电压保护######################*/
    
    
    /*#############高温保护######################*/
    extern TemperatureSensor_t Temperature_sensor;       // 温度传感器
    HXC::NVS_DATA<bool> default_high_temperature_protect_state("temp_p_sta",false);// 默认电流保护状态
    HXC::NVS_DATA<float> high_temperature_protect_value("temp_protect_value",60);// 温度保护值,℃
    // 高温保护任务
    HXC::thread<void> temperature_protect_thread([](){
        while (true){
            if(Temperature_sensor.getTemperature()>high_temperature_protect_value){
                power_output.off();
                buzz.buzz(0.5);
                delay(5000);
            }
            delay(10);
        }
    });

    // 开关高温保护
    void temperature_protect_ctrl(bool state) {
        if(state){
            temperature_protect_thread.start(/*taskname=*/"temperature_protect",/*stacksize=*/2048);// 高温保护任务启动
        } else {
            temperature_protect_thread.stop();
        }
    }

    // 保护初始化
    void protect_init(){
        current_protect_ctrl(default_current_protect_state.read());// 电流保护初始化
        voltage_protect_ctrl(default_low_voltage_protect_state.read());// 电压保护初始化
        temperature_protect_ctrl(default_high_temperature_protect_state.read());// 温度保护初始化
    }
}
#endif