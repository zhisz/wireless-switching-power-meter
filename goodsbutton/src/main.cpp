/*
 * @LastEditors: qingmeijiupiao
 * @Description: 
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 19:43:43
 */
#include <Arduino.h>
#include "PowerCtrl.hpp"
#include "shell.hpp"
#include <tuple>
//NVS数据，用于esp_now数据包密钥
HXC::NVS_DATA <uint16_t> esp_now_secret_key("secret_key",DEFAULT_SECRET_KEY);

//重启
void resret(){
    ESP.restart();
}

/**
 * @brief : LED闪烁任务
 * @return  {*}
 * @param {tuple<uint32_t,float>} 参数类型
 * @param {uint32_t} 闪烁次数
 * @param {float} 闪烁频率
 */
HXC::thread<std::tuple<uint32_t,float>> LED_blink_task([](std::tuple<uint32_t,float> arg){
    uint32_t count=std::get<0>(arg);
    float led_HZ=std::get<1>(arg);
    for(int i=0;i<count;i++){
        digitalWrite(10,LOW);
        delay(500.f/HZ);
        digitalWrite(10,HIGH);
        delay(500.f/HZ);
    }
});

void setup() {

    pinMode(0,INPUT_PULLUP);
    pinMode(10,OUTPUT);
    pinMode(2,INPUT_PULLDOWN);


    change_secret_key(esp_now_secret_key.read());
    PowerCtrl::setup();

    if(!digitalRead(2)){//是否插入USB
        PowerCtrl::power_off();
        digitalWrite(10,HIGH);
        int press_time=0;
        bool is_send=false;
        while (!digitalRead(0)){
            delay(10);
            press_time+=10;
            if(press_time>1000&&!is_send){
                PowerCtrl::power_on();
                //LED闪烁
                LED_blink_task.start(std::make_tuple(/*闪烁次数=*/3,/*闪烁频率=*/5),"LED_blink_task",512);
                LED_blink_task.join();//等待线程结束
                is_send=true;
            }
            if(press_time>10000){
                //发送配对
                PowerCtrl::send_pair_package();
                //LED闪烁
                LED_blink_task.start(std::make_tuple(/*闪烁次数=*/10,/*闪烁频率=*/10),"LED_blink_task",512);
                LED_blink_task.join();//等待线程结束
            }
        }
        digitalWrite(10,LOW);
        esp_deep_sleep_enable_gpio_wakeup(1<<GPIO_NUM_0,ESP_GPIO_WAKEUP_GPIO_LOW);//开启GPIO0中断
        esp_deep_sleep_enable_gpio_wakeup(1<<GPIO_NUM_2,ESP_GPIO_WAKEUP_GPIO_HIGH);//开启GPIO2中断
        esp_deep_sleep_start();//进入睡眠
    }else{
        digitalWrite(10,HIGH);
        Serial.begin(115200);
        attachInterrupt(2, resret, FALLING);
        attachInterrupt(0,PowerCtrl::power_off,FALLING);
        SHELL::shell_thread.start("SHELL",2048);

    }
}

void loop() {}

