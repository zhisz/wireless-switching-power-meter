#ifndef WIRELESSCTRL_HPP
#define WIRELESSCTRL_HPP
#include <Arduino.h>
#include "POWERMETER.hpp"
#include "powerctrl.hpp"
#include "NVSSTORAGE.hpp"
#include "ESPNOW.hpp"
extern POWERCTRL_t power_output;
uint8_t self_Macaddress[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};  // 自己的mac地址,默认广播地址

namespace WIRELESSCTRL {
    
    void pair_func(data_package receive_data){
        uint8_t* pair_mac;
        memcpy(pair_mac, receive_data.data.data(),6);
        NVSSTORAGE::pair_mac[0]=pair_mac[0];
        NVSSTORAGE::pair_mac[1]=pair_mac[1];
        NVSSTORAGE::pair_mac[2]=pair_mac[2];
        NVSSTORAGE::pair_mac[3]=pair_mac[3];
        NVSSTORAGE::pair_mac[4]=pair_mac[4];
        NVSSTORAGE::pair_mac[5]=pair_mac[5];
        NVSSTORAGE::NVS_save();
    }

    // 心跳控制相关
    int Heartbeat_var=0;
    TaskHandle_t Heartbeat_task_handle = nullptr;
    int HeartbeatFRC = 10;
    int8_t Heartbeat_max_err=5;


    // 心跳任务
    void Heartbeat_task(void * pvParameters) {
        while (true) {
            if(Heartbeat_var>Heartbeat_max_err*1000/HeartbeatFRC){//心跳超时
                power_output.off();// 关闭电源
            }
            delay(1000/HeartbeatFRC);// 延时
            Heartbeat_var+=1000/HeartbeatFRC;// 增加心跳
        }
    }
    void Heartbeat_func(data_package receive_data){
        if(Heartbeat_task_handle==nullptr){// 创建心跳任务
            xTaskCreatePinnedToCore(Heartbeat_task, "Heartbeat_task", 2048, NULL, 5, &Heartbeat_task_handle, 1);
        }
        uint8_t* data;
        memcpy(data, receive_data.data.data(),receive_data.data.size());
        int frc=*(int*)data;
        int max_err=*(int*)(data+4);
        if(frc!=0&&frc!=HeartbeatFRC){
            HeartbeatFRC=frc;
        }
        if(max_err!=-1&&max_err!=Heartbeat_max_err){
            Heartbeat_max_err=max_err;
        }
        Heartbeat_var=0;// 重置心跳
    }
    // 关闭心跳控制
    void close_heartbeat(data_package receive_data){
        if(Heartbeat_task_handle!=nullptr){
            vTaskDelete(Heartbeat_task_handle);
            Heartbeat_task_handle = nullptr;
        }
    }
    // 电源主动控制
    void power_ctrl(data_package receive_data) {
        bool state = *(bool*)receive_data.data.data();
        if (state) {
            power_output.on();
        } else {
            power_output.off();
        }
    }

    // 发送数据结构体
    struct Power_data{
        bool now_state;
        float voltage;
        float current;
        float mah;
        float mwh;
        int time;
    };
    int send_data_frc=10;
    TaskHandle_t send_data_task_handle = nullptr;
    void send_data_task( void * pvParameters ) {
        while (true) {
            Power_data send_power_data_package={
                power_output.getstate(),
                POWERMETER::voltage,
                POWERMETER::current,
                POWERMETER::output_mah,
                POWERMETER::output_mwh,
                millis()
            };
            uint8_t send_power_data_package_array[sizeof(Power_data)];
            memcpy(send_power_data_package_array, &send_power_data_package, sizeof(Power_data));
            esp_now_send_package("Power_data",send_power_data_package_array,sizeof(Power_data));
            delay(1000/send_data_frc);
        }
    }
    void send_data_ctrl(data_package receive_data) {
        uint8_t* data;
        memcpy(data, receive_data.data.data(),receive_data.data.size());
        bool is_continue = *(bool*)data;
        int frc=*(int*)(data+4);
        if(frc!=0&&frc!=send_data_frc){
            send_data_frc=frc;
        }
        if(is_continue&&send_data_task_handle==nullptr){
            xTaskCreatePinnedToCore(send_data_task, "send_data_task", 2048, NULL, 5, &send_data_task_handle, 1);
        }else{
            if(send_data_task_handle!=nullptr){
                vTaskDelete(send_data_task_handle);
                send_data_task_handle = nullptr;
            }
            Power_data send_power_data_package={
                power_output.getstate(),
                POWERMETER::voltage,
                POWERMETER::current,
                POWERMETER::output_mah,
                POWERMETER::output_mwh,
                millis()
            };
            uint8_t send_power_data_package_array[sizeof(Power_data)];
            memcpy(send_power_data_package_array,&send_power_data_package,sizeof(Power_data));
            esp_now_send_package("Power_data",send_power_data_package_array,sizeof(Power_data),NVSSTORAGE::pair_mac);
        }
    }
    // 无线控制初始化
    void wireless_ctrl_setup() {
        WiFi.macAddress(self_Macaddress);
        esp_now_setup();
        callback_map["Heartbeatpackage"]=Heartbeat_func;
        callback_map["pair"]=pair_func;
        callback_map["close_heartbeat"]=close_heartbeat;
        callback_map["power_ctrl"]=power_ctrl;
    }

}
#endif