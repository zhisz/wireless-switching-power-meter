/*
 * @LastEditors: qingmeijiupiao
 * @Description: 无线急停功率计控制封装
 * @Author: qingmeijiupiao
 * @Date: 2024-10-15 13:55:10
 */
#ifndef POWERCTRL_HPP
#define POWERCTRL_HPP
#include "ESPNOW.hpp"
#include "HXC_NVS.hpp"

//无线急停功率计
namespace PowerCtrl{
    //NVS数据，存储配对的MAC地址
    HXC::NVS_DATA<MAC_t>pair_mac("pair_mac",MAC_t(0XFF,0XFF,0XFF,0XFF,0XFF,0XFF));
    void power_state_reciver(HXC_ESPNOW_data_pakage receive_data);
    static bool state = false;
    static bool is_new_data=false;
    //功率数据结构
    struct Power_data_t{
        bool now_state;
        float voltage;
        float current;
        float mah;
        float mwh;
        uint32_t runtime;
    }power_data;

    //请求配对
    void send_pair_package(){
        uint8_t self_mac[6];
        WiFi.macAddress(self_mac);
        esp_now_send_package("pair",self_mac,6,broadcastMacAddress);

    }
    //开启功率计输出
    void power_on(){
        bool state = true;
        esp_now_send_package("power_ctrl",(uint8_t*)(&state),1,pair_mac);
    };
    //关闭功率计输出
    void power_off(){
        bool state = false;
        esp_now_send_package("power_ctrl",(uint8_t*)(&state),1,pair_mac);
    };
    //获取功率计开关状态
    bool getstate(){
        esp_now_send_package("get_power_state",nullptr,0,pair_mac);
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
    //控制功率计发送数据
    void ctrl_send_data(bool is_continue,int frc=0){
        bool state = is_continue;
        int _frc = frc;
        uint8_t data[5];
        data[0] = state;
        memcpy(data+1,&_frc,sizeof(int));
        esp_now_send_package("send_data_ctrl",data,sizeof(bool)+sizeof(int),pair_mac);
    }
    //功率状态回调
    void power_state_reciver(HXC_ESPNOW_data_pakage receive_data){
        bool _state = *(bool*)receive_data.data;
        PowerCtrl::state = _state;
        PowerCtrl::is_new_data = true;
    }
    //配对回调
    void paircallback(HXC_ESPNOW_data_pakage receive_data){
        add_esp_now_peer_mac(receive_data.data);
        //保存数据到NVS
        pair_mac=receive_data.data;
    }
    //接收功率数据回调函数
    void Power_data_callback(HXC_ESPNOW_data_pakage receive_data){
        memcpy(&power_data,receive_data.data,sizeof(Power_data_t));
    }
    //发送心跳包
    void send_Heartbeat(int _frc=10,int max_error=3){
        uint8_t data[8];
        memcpy(data,&_frc,sizeof(int));
        memcpy(data+4,&max_error,sizeof(int));
        esp_now_send_package("Heartbeatpackage",data,sizeof(int)+sizeof(int),pair_mac);
    }
    //初始化
    void setup(){
        esp_now_setup(pair_mac);
        add_esp_now_callback("Power_state",power_state_reciver);
        add_esp_now_callback("pair",paircallback);
        add_esp_now_callback("Power_data",Power_data_callback);
    }

};




#endif