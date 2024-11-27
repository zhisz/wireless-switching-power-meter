/*
 * @LastEditors: qingmeijiupiao
 * @Description: 按键类
 * @Author: qingmeijiupiao
 * @Date: 2024-11-07 15:41:37
 */

#ifndef Button_HPP
#define Button_HPP
#include <Arduino.h>
#include <functional>
#include "HXCthread.hpp"
//按键
class Button{
public:
    Button(uint8_t Pin,bool on_press_state=0){
        pin = Pin;
        press_state=on_press_state;
    }
    ~Button(){

    };
    bool get_state(){
        return state;
    }
    void set_freq(int _freq){
        this->freq=_freq;
    }
        
    void setup(){
        pinMode(pin,INPUT_PULLUP);
        state = digitalRead(pin);
        callback_thread.start(this,"Button_callback_thread");
    }
    void add_press_callback(std::function<void(void)> press_callback){
        on_press_callback_function = press_callback;
    }
    void add_release_callback(std::function<void(void)> release_callback){
        on_release_callback_function = release_callback;
    }
    void add_change_callback(std::function<void(void)> change_callback){
        on_change_callback_function = change_callback;
    }
protected:
    void callback(){
        bool temp_state = digitalRead(pin);
        if(temp_state!=state){//有变化
            state = temp_state;
            if(temp_state==press_state){//按下
                if(on_press_callback_function!=nullptr){
                    on_press_callback_function();
                }
            }else{//释放
                if(on_release_callback_function!=nullptr){
                    on_release_callback_function();
                }
            }
        }      

    }
    HXC::thread<Button*> callback_thread = HXC::thread<Button*>([](Button* Button_instance){
        while(1){
            Button_instance->callback();
            delay(1000/Button_instance->freq);
        }
    });

    std::function<void(void)> on_press_callback_function=nullptr;
    std::function<void(void)> on_release_callback_function=nullptr;
    std::function<void(void)> on_change_callback_function=nullptr;
    bool press_state;
    uint8_t pin;
    bool state;
    uint16_t freq=100;
};

#endif