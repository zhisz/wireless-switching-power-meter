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

extern POWERCTRL_t power_output;
namespace OTHER_FUNCTION{
    /*#############串口打印######################*/
    // 串口打印数据
    TaskHandle_t serial_print_handle = nullptr;
    void serial_print_data_task(void* pvParameters) {
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
    }
    // 串口打印数据控制
    void serial_print_ctrl(bool state) {
        if (state&&serial_print_handle==nullptr) {
            xTaskCreate(serial_print_data_task, "serial_print_data", 2048, NULL, 5, &serial_print_handle);
        }else{
            if(serial_print_handle!=nullptr){
                vTaskDelete(serial_print_handle);
                serial_print_handle = nullptr;
            }
        }
    }
    /*#############串口打印######################*/

    /*#############电流保护######################*/
    HXC::NVS_DATA<bool> default_current_protect_state("current_p_sta",false);// 默认电流保护状态

    HXC::NVS_DATA<float> current_protect_value("current_value",40);// 电流保护值,A
    TaskHandle_t current_protect_handle = nullptr;
    void current_protect_task(void* pvParameters) {
        while (true)
        {   
            if(POWERMETER::current>current_protect_value){
                power_output.off();
                buzz.buzz(0.5);
                delay(5000);
            }
            delay(2);
        }
    }
    // 开关电流保护
    void current_protect_ctrl(bool state) {
        if (state&&current_protect_handle==nullptr) {
            xTaskCreate(current_protect_task, "current_protect", 2048, NULL, 5, &current_protect_handle);
        }else{
            if(current_protect_handle!=nullptr){
                vTaskDelete(current_protect_handle);
                current_protect_handle = nullptr;
            }
        }
    }
    /*#############电流保护######################*/


    /*#############低电压保护######################*/
    HXC::NVS_DATA<bool> default_low_voltage_protect_state("voltage_p_sta",false);// 默认电流保护状态
    HXC::NVS_DATA<float> low_voltage_protect_value("low_voltage_value",12);// 电压保护值,V
    TaskHandle_t voltage_protect_task_handle= nullptr;
    void voltage_protect_task(void* p){
        while (true){
            if(POWERMETER::voltage<low_voltage_protect_value){
                power_output.off();
                buzz.buzz(0.5);
                delay(5000);
            }
            delay(10);
        }
    }
    // 开关低压保护
    void voltage_protect_ctrl(bool state) {
        if (state&&voltage_protect_task_handle==nullptr) {
            xTaskCreate(voltage_protect_task, "voltage_protect", 8192, NULL, 5, &voltage_protect_task_handle);
        }else{
            if(voltage_protect_task_handle!=nullptr){
                vTaskDelete(voltage_protect_task_handle);
                voltage_protect_task_handle = nullptr;
            }
        }
    }
    /*#############低电压保护######################*/
    
    
    /*#############高温保护######################*/
    extern TemperatureSensor_t Temperature_sensor;       // 温度传感器
    HXC::NVS_DATA<bool> default_high_temperature_protect_state("temp_p_sta",false);// 默认电流保护状态
    HXC::NVS_DATA<float> high_temperature_protect_value("temp_protect_value",60);// 温度保护值,℃
    TaskHandle_t temperature_protect_task_handle= nullptr;
    void temperature_protect_task(void* p){
        while (true){
            if(Temperature_sensor.getTemperature()>high_temperature_protect_value){
                power_output.off();
                buzz.buzz(0.5);
                delay(5000);
            }
            delay(10);
        }
    }
    // 开关高温保护
    void temperature_protect_ctrl(bool state) {
        if (state&&temperature_protect_task_handle==nullptr) {
            xTaskCreate(temperature_protect_task, "temperature_protect", 2048, NULL, 5, &temperature_protect_task_handle);
        }else{
            if(temperature_protect_task_handle!=nullptr){
                vTaskDelete(temperature_protect_task_handle);
                temperature_protect_task_handle = nullptr;
            }
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