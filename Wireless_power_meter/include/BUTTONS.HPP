/*
 * @LastEditors: qingmeijiupiao
 * @Description: 按键相关
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 17:45:29
 */
#ifndef BUTTONS_HPP
#define BUTTONS_HPP
#include "static/PINS.h"
#include "OneButton.h"
#include "SCREEN.hpp"
OneButton right_button(BUT1, true);           // 右按键
OneButton big_button(BUT2, true);             // 大按键
OneButton left_button(BUT3, true);            // 左按键


// 按键控制相关
namespace BUTTON {
    // 按键任务
    void button_task(void * pvParameters) {
        pinMode(BUT1, INPUT_PULLUP);                               // 设置按键为上拉输入模式
        pinMode(BUT2, INPUT_PULLUP);
        pinMode(BUT3, INPUT_PULLUP);

        // 定义按下中间按键时的操作（关闭电源）
        auto pressed_func = []() {
            power_output.off();
        };

        // 定义长按中间按键时的操作（打开电源）
        auto longpress_func = []() {
            power_output.on();
        };

        // 定义左按键按下时的操作（切换到上一个页面）
        auto left_button_pressed_func = []() {
            if (SCREEN::now_page == SCREEN::page_list.begin()) {
                SCREEN::now_page = --SCREEN::page_list.end(); // 如果当前是第一个页面，跳到最后一个页面
            } else {
                --SCREEN::now_page;
            }
        };

        auto left_button_long_pressed_func = []() {
            if(SCREEN::screen_rotation == 3) {
                SCREEN::screen_rotation = 1;
            }else {
                SCREEN::screen_rotation=3;
            }
            SCREEN::tft.setRotation(SCREEN::screen_rotation);
        };

        // 定义右按键按下时的操作（切换到下一个页面）
        auto right_button_pressed_func = []() {
            if (SCREEN::now_page == --SCREEN::page_list.end()) { // 如果当前是最后一个页面，跳到第一个页面
                SCREEN::now_page = SCREEN::page_list.begin();
            } else {
                ++SCREEN::now_page;
            }
        };

        // 给按键绑定相应的功能
        big_button.attachPress(pressed_func);
        big_button.attachLongPressStart(longpress_func);
        big_button.setLongPressIntervalMs(1000); // 设置长按时间为1秒

        left_button.attachClick(left_button_pressed_func); // 左按键点击切换页面
        left_button.attachLongPressStart(left_button_long_pressed_func);// 左按键长按旋转屏幕
        left_button.setLongPressIntervalMs(1000);

        right_button.attachClick(right_button_pressed_func); // 右按键点击切换页面

        while (true) {
            left_button.tick(); // 检测按键状态
            right_button.tick();
            big_button.tick();
            delay(1); // 每次循环延时2毫秒
        }
    }
}



#endif