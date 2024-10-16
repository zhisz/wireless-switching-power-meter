/*
 * @LastEditors: qingmeijiupiao
 * @Description: 
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 19:43:43
 */
#include <Arduino.h>
#include "PowerCtrl.hpp"
#include "NVSSTORAGE.hpp"
void setup() {
    Serial.begin(115200);
    NVSSTORAGE::NVS_read();
    PowerCtrl::setup();
    PowerCtrl::power_off();
    pinMode(9,INPUT);
    pinMode(10,OUTPUT);
    digitalWrite(10,HIGH);
    delay(300);
    //HOLD
    // esp_deep_sleep_enable_gpio_wakeup(1<<GPIO_NUM_9,ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_deep_sleep_start();

}

void loop() {
}

