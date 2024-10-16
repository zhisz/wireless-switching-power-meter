/*
 * @LastEditors: qingmeijiupiao
 * @Description: 
 * @Author: qingmeijiupiao
 * @Date: 2024-10-15 13:55:10
 */
#ifndef POWERCTRL_HPP
#define POWERCTRL_HPP
#include "ESPNOW.hpp"
#include "NVSSTORAGE.hpp"
void power_state_reciver(data_package receive_data);
namespace PowerCtrl{
    bool state = false;
    bool is_new_data=false;
    struct Power_data{
        bool now_state;
        float voltage;
        float current;
        float mah;
        float mwh;
        uint32_t runtime;
    }power_data;
    void send_pair_package(){
        uint8_t* self_mac;
        WiFi.macAddress(self_mac);
        esp_now_send_package("pair",self_mac,6,NVSSTORAGE::pair_mac);

    }
    void power_on(){
        bool state = true;
        esp_now_send_package("power_ctrl",(uint8_t*)(&state),1,NVSSTORAGE::pair_mac);
    };
    void power_off(){
        bool state = false;
        esp_now_send_package("power_ctrl",(uint8_t*)(&state),1,NVSSTORAGE::pair_mac);
    };
    bool getstate(){
        esp_now_send_package("get_power_state",nullptr,0,NVSSTORAGE::pair_mac);
        int max_delay = 300;//最大等待时间（ms）
        while(!is_new_data){
            max_delay--;
            if(max_delay==0){
                break;
            }
            delay(1);
        };
        is_new_data=false;
        return state;
    }
    void ctrl_send_data(bool is_continue,int frc=0){
        bool state = is_continue;
        int _frc = frc;
        uint8_t data[5];
        data[0] = state;
        memcpy(data+1,&_frc,sizeof(int));
        esp_now_send_package("send_data_ctrl",data,sizeof(bool)+sizeof(int),NVSSTORAGE::pair_mac);
    }
    void power_state_reciver(data_package receive_data){
        bool _state = *(bool*)receive_data.data;
        PowerCtrl::state = _state;
        PowerCtrl::is_new_data = true;
    }
    void paircallback(data_package receive_data){
        if(receive_data.package_name=="pair"){
            memcpy(NVSSTORAGE::pair_mac,receive_data.data,6);
            NVSSTORAGE::NVS_save();
        }
    }
    void Power_data_callback(data_package receive_data){
        memcpy(&power_data,receive_data.data,sizeof(Power_data));
    }
    void setup(){
        esp_now_setup(NVSSTORAGE::pair_mac);
        callback_map["Power_state"]=power_state_reciver;
        callback_map["pair"]=paircallback;
        callback_map["Power_data"]=Power_data_callback;
    }

};



#endif