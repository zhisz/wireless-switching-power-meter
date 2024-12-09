/*
 * @LastEditors: qingmeijiupiao
 * @Description: 断电保存数据相关
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 17:21:48
 */
#ifndef NVSSTORAGE_HPP
#define NVSSTORAGE_HPP
#include "Preferences.h"
#include "ESPNOW.hpp"
//将数据保存在NVS存储中,断电保存
namespace NVSSTORAGE {
    Preferences preferences;
    MAC_t pair_mac(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
    //esp_now密钥
    uint16_t esp_now_secret_key;
    //采样电阻阻值,单位mΩ
    float sample_resistance;
    void NVS_read() {
        preferences.begin("DATA", true);
        pair_mac.mac[0]=preferences.getUChar("mac1",0xFF);
        pair_mac.mac[1]=preferences.getUChar("mac2",0xFF);
        pair_mac.mac[2]=preferences.getUChar("mac3",0xFF);
        pair_mac.mac[3]=preferences.getUChar("mac4",0xFF);
        pair_mac.mac[4]=preferences.getUChar("mac5",0xFF);
        pair_mac.mac[5]=preferences.getUChar("mac6",0xFF);
        sample_resistance=preferences.getFloat("sample_resistance",2.0);
        esp_now_secret_key=preferences.getUShort("esp_now_secret_key",DEFAULT_SECRET_KEY);
        change_secret_key(esp_now_secret_key);
        preferences.end();
    }

    void NVS_save() {
        preferences.begin("DATA", false);
        preferences.putUChar("mac1", pair_mac.mac[0]);
        preferences.putUChar("mac2", pair_mac.mac[1]);
        preferences.putUChar("mac3", pair_mac.mac[2]);
        preferences.putUChar("mac4", pair_mac.mac[3]);
        preferences.putUChar("mac5", pair_mac.mac[4]);
        preferences.putUChar("mac6", pair_mac.mac[5]);
        preferences.putFloat("sample_resistance", sample_resistance);
        preferences.putUShort("esp_now_secret_key", esp_now_secret_key);
        preferences.end();
    }
}
#endif //NVSSTORAGE_HPP