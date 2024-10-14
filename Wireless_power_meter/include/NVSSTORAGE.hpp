/*
 * @LastEditors: qingmeijiupiao
 * @Description: 
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 17:21:48
 */
#ifndef NVSSTORAGE_HPP
#define NVSSTORAGE_HPP
#include "Preferences.h"
namespace NVSSTORAGE {
    Preferences preferences;
    uint8_t pair_mac[6];
    void NVS_read() {
        preferences.begin("DATA", true);
        pair_mac[0]=preferences.getUChar("mac1",0xFF);
        pair_mac[1]=preferences.getUChar("mac2",0xFF);
        pair_mac[2]=preferences.getUChar("mac3",0xFF);
        pair_mac[3]=preferences.getUChar("mac4",0xFF);
        pair_mac[4]=preferences.getUChar("mac5",0xFF);
        pair_mac[5]=preferences.getUChar("mac6",0xFF);
        preferences.end();
    }

    void NVS_save() {
        preferences.begin("DATA", false);
        preferences.putUChar("mac1", pair_mac[0]);
        preferences.putUChar("mac2", pair_mac[1]);
        preferences.putUChar("mac3", pair_mac[2]);
        preferences.putUChar("mac4", pair_mac[3]);
        preferences.putUChar("mac5", pair_mac[4]);
        preferences.putUChar("mac6", pair_mac[5]);
        preferences.end();
    }
}
#endif //NVSSTORAGE_HPP