/*
 * @LastEditors: qingmeijiupiao
 * @Description: 
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 19:43:43
 */
#include <Arduino.h>
#include "PowerCtrl.hpp"

void resret(){
    ESP.restart();
}
void setup() {

    pinMode(0,INPUT_PULLUP);
    pinMode(10,OUTPUT);
    pinMode(2,INPUT_PULLDOWN);
    
    PowerCtrl::setup();

    if(!digitalRead(2)){
        PowerCtrl::power_off();
        digitalWrite(10,HIGH);
        int press_time=0;
        bool is_send=false;
        while (!digitalRead(0)){
            delay(10);
            press_time+=10;
            if(press_time>1000&&!is_send){
                PowerCtrl::power_on();
                for(int i=0;i<3;i++){
                    digitalWrite(10,LOW);
                    delay(100);
                    digitalWrite(10,HIGH);
                    delay(100);
                }

                is_send=true;
            }
            if(press_time>10000){
                PowerCtrl::send_pair_package();
                for(int i=0;i<10;i++){
                    digitalWrite(10,LOW);
                    delay(50);
                    digitalWrite(10,HIGH);
                    delay(50);
                }
            }
        }
        digitalWrite(10,LOW);
        esp_deep_sleep_enable_gpio_wakeup(1<<GPIO_NUM_0,ESP_GPIO_WAKEUP_GPIO_LOW);
        esp_deep_sleep_enable_gpio_wakeup(1<<GPIO_NUM_2,ESP_GPIO_WAKEUP_GPIO_HIGH);
        esp_deep_sleep_start();
    }else{
        digitalWrite(10,HIGH);
        Serial.begin(115200);
        attachInterrupt(2, resret, FALLING);
        attachInterrupt(0,PowerCtrl::power_off,FALLING);
        
        PowerCtrl::send_pair_package();

        PowerCtrl::ctrl_send_data(true,50);
        
    }
}

void loop() {
    Serial.print(PowerCtrl::power_data.voltage);
    Serial.print(",");
    Serial.println(PowerCtrl::power_data.current);
    delay(10);
}

