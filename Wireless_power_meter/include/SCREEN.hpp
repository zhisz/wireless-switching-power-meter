/*
 * @LastEditors: qingmeijiupiao
 * @Description: 
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 17:24:35
 */
#ifndef SCREEN_HPP
#define SCREEN_HPP
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <list> 
#include "pic/wifi_pic.hpp"
#include "NVSSTORAGE.hpp"
#include "POWERMETER.hpp"
#include "powerctrl.hpp"
#include "WIRELESSCTRL.hpp"
#include "TemperatureSensor.hpp"
extern TemperatureSensor_t Temperature_sensor; 
extern POWERCTRL_t power_output;

// 格式化浮点数为固定长度的字符串
void sprintf_float(double value, char* buffer, int len) {
    int max = pow(10, len) - 1;               // 最大值，根据长度决定
    if (value > max) value = max;
    if (value < 0) value = 0;
    
    int count = 0;
    int temp = value;                            // 记录小数点前的位数
    while (temp > 10) {
        temp /= 10;
        count++;
    }
    
    // 构建格式化字符串
    String format = "%";
    if (count == len) {
        format += "d";
    } else {
        format += "." + String(len - count) + "f";
    }
    
    sprintf(buffer, format.c_str(), value);   // 格式化浮点数
}

// 屏幕显示相关
namespace SCREEN {
    const int SCREEN_HZ = 60;                 // 屏幕刷新率

    TFT_eSPI tft = TFT_eSPI();                // 创建屏幕对象
    TFT_eSprite clk = TFT_eSprite(&tft);      // 创建缓冲区

    // 主页面绘制函数
    void mian_page() {
        clk.setTextColor(TFT_WHITE);          // 设置文本颜色为白色
        
        // 电压部分
        clk.setTextColor(TFT_RED);  
        clk.setCursor(10, 10);
        char buffer[4];
        sprintf_float(POWERMETER::voltage, buffer, 3); // 格式化电压数据
        clk.setTextFont(6);
        clk.setTextSize(1);
        clk.print(buffer);
        clk.setTextFont(1);
        clk.setTextSize(5);
        clk.print("V");

        // 电流部分
        clk.setTextColor(TFT_GREEN);  
        sprintf_float(POWERMETER::current, buffer, 3); // 格式化电流数据
        clk.setCursor(10, 60);
        clk.setTextFont(6);
        clk.setTextSize(1);
        clk.print(buffer);
        clk.setTextFont(1);
        clk.setTextSize(5);
        clk.print("A");
        
        // 功率部分
        clk.setTextColor(TFT_BLUE); 
        sprintf_float(POWERMETER::voltage*POWERMETER::current, buffer, 3); // 格式化输出电量
        clk.setCursor(10, 110);
        clk.setTextFont(4);
        clk.setTextSize(1);
        clk.print(buffer);
        clk.print("W");

        clk.setTextColor(TFT_WHITE); 
       //当前开关状态部分“<——”
        auto draw_arrow = []() {//lamda方便编辑器折叠
            uint16_t color=power_output.getstate()?TFT_GREEN:TFT_RED;//设置颜色，关是红色，开是绿色
            clk.drawLine(104, 120-1,120, 120-11, color);//画箭头上
            clk.drawLine(104, 120,120, 120-10, color);

            clk.drawLine(105, 119,150, 119, color);
            clk.drawLine(105, 120,150, 120, color);//画箭头中心
            clk.drawLine(105, 121,150, 121, color);

            clk.drawLine(104, 120,120, 120+10, color);//画箭头下
            clk.drawLine(104, 120+1,120, 120+11, color);
            if(!power_output.getstate()){//如果是关的话
                //画个X
                clk.drawLine(135-10, 120+11,135+10, 120-11, color);
                clk.drawLine(135-10, 120-11,135+10, 120+11, color);
            }
        };draw_arrow();

        // 绘制温度
        clk.setCursor(170, 10);
        clk.setTextFont(4);
        clk.setTextSize(1);
        clk.print(int(Temperature_sensor.getTemperature())); // 显示温度
        clk.print("  C"); // 显示单位摄氏度
        clk.drawCircle(203, 14, 3, TFT_WHITE); // 显示小圆圈作为摄氏度符号

        // 最大电压和电流显示
        clk.setCursor(170, 30);
        clk.print("MAX:");
        clk.setCursor(170, 50);
        clk.setTextColor(TFT_RED);
        sprintf_float(POWERMETER::MAX_VOLTAGE, buffer, 2);
        clk.print(buffer);
        clk.setTextColor(TFT_WHITE);
        clk.print("V");

        clk.setCursor(170, 70);
        clk.setTextColor(TFT_GREEN);
        sprintf_float(POWERMETER::MAX_CURRENT, buffer, 2);
        clk.print(buffer);
        clk.setTextColor(TFT_WHITE);
        clk.print("A");

        //连接图标
        if(is_conect){
            clk.pushImage(180, 90, __WIFI_WIDTH, __WIFI_HEIGHT, __wifi);
        }else{
            clk.pushImage(180, 90, __CLOSE_WIFI_WIDTH, __CLOSE_WIFI_HEIGHT, __close_wifi);
        }
        // 绘制分割线
        for (int i = 0; i < 3; i++) {
            clk.drawLine(162 + i, 0, 162 + i, 135, TFT_YELLOW); // 分割线用黄色
        }
    }

    // 累计电量页面绘制函数
    void amperage_page() {
        clk.setCursor(0, 15);
        char buffer[4];
        sprintf_float(POWERMETER::output_mah, buffer, 4); // 显示累计毫安时
        clk.setTextFont(6);
        clk.setTextSize(1);
        clk.print(buffer);
        clk.setTextFont(1);
        clk.setTextSize(5);
        clk.print("mAH");

        sprintf_float(POWERMETER::output_mwh, buffer, 4); // 显示累计毫瓦时
        clk.setCursor(0, 60);
        clk.setTextFont(6);
        clk.setTextSize(1);
        clk.print(buffer);
        clk.setTextFont(1);
        clk.setTextSize(5);
        clk.print("mWH");

        // 显示累计时间
        int time_m = POWERMETER::last_time / 60000000;
        int time_s = (POWERMETER::last_time - time_m * 60000000) / 1e6;
        int time_ms = (POWERMETER::last_time - time_m * 60000000 - time_s * 1e6) / 1000;
        clk.setCursor(20, 110);
        clk.setTextFont(4);
        clk.setTextSize(1);
        clk.print("Runtime:  ");
        sprintf(buffer, "%4d", time_m);
        clk.print(time_m);
        clk.print(":");
        sprintf(buffer, "%2d", time_s);
        clk.print(buffer);
        clk.print(":");
        sprintf(buffer, "%2d", time_ms);
        buffer[2]='\0';
        clk.print(buffer);
    }
    
    
    //曲线绘制
    static uint8_t power_curve_point[190];
    static uint8_t voltage_curve_point[190];
    static uint8_t current_curve_point[190];
    // 曲线页面
    void curve_page() {
        // 曲线低点高度
        const float curve_mini_height=0.1;//0-1
        // 曲线高点高度
        const float curve_maxi_height=0.8;//0-1


        auto background = []() {
            for(int i=50;i<240;i+=10){
                clk.drawLine(i, 0, i, 135, 0x3333);
            }
            for (int i = 0; i < 135; i+=10)
            {
                clk.drawLine(50, i, 240, i, 0x3333);
            }
            
        };background();
        //MAX部分
        clk.setTextColor(TFT_WHITE);
        clk.setCursor(3, 0);
        clk.setTextFont(2);
        clk.setTextSize(1);
        clk.print("MAX->\n");
        clk.setTextColor(TFT_RED);
        clk.setCursor(3, 12);
        clk.print(POWERMETER::voltage_queue.get_max());
        clk.print("V\n");
        clk.setTextColor(TFT_GREEN);
        clk.setCursor(3, 24);
        clk.print(POWERMETER::current_queue.get_max());
        clk.print("A\n");
        clk.setTextColor(TFT_BLUE);
        clk.setCursor(3, 36);
        clk.print(POWERMETER::power_queue.get_max());
        clk.print("W\n");

        //MIN部分
        clk.setTextColor(TFT_WHITE);
        clk.setCursor(3, 135-16);
        clk.setTextFont(2);
        clk.setTextSize(1);
        clk.print("MIN->\n");
        clk.setTextColor(TFT_RED);
        clk.setCursor(3, 70+12);
        clk.print(POWERMETER::voltage_queue.get_min());
        clk.print("V\n");
        clk.setTextColor(TFT_GREEN);
        clk.setCursor(3, 70+24);
        clk.print(POWERMETER::current_queue.get_min());
        clk.print("A\n");
        clk.setTextColor(TFT_BLUE);
        clk.setCursor(3, 70+36);
        clk.print(POWERMETER::power_queue.get_min());
        clk.print("W\n");


        float step=POWERMETER::READ_HZ*POWERMETER::data_save_time/190;
        memset(power_curve_point,0,sizeof(power_curve_point));
        memset(voltage_curve_point,0,sizeof(voltage_curve_point));
        memset(current_curve_point,0,sizeof(current_curve_point));
        
        //曲线缩放
        const float curve_scale=1/(curve_maxi_height-curve_mini_height);
        //保存为采样点
        for(int i=0;i<190;i++){
            if(POWERMETER::power_queue.get_max()-POWERMETER::power_queue.get_min()!=0){
                float mini_var=POWERMETER::power_queue.get_min();
                float data_range=POWERMETER::MAX_CURRENT*POWERMETER::MAX_VOLTAGE-mini_var;
                power_curve_point[i]=135*(POWERMETER::power_queue.toArray()[int(i*step)]-mini_var)/(curve_scale*data_range);
            }

            if(POWERMETER::voltage_queue.get_max()-POWERMETER::voltage_queue.get_min()!=0){
                float mini_var=POWERMETER::voltage_queue.get_min();
                float data_range = POWERMETER::MAX_VOLTAGE-mini_var;
                voltage_curve_point[i]=135*(POWERMETER::voltage_queue.toArray()[int(i*step)]-mini_var)/(curve_scale*data_range);
            }
            if(POWERMETER::current_queue.get_max()-POWERMETER::current_queue.get_min()!=0){
                float mini_var=POWERMETER::current_queue.get_min();
                float data_range = POWERMETER::MAX_CURRENT-mini_var;
                current_curve_point[i]=135*(POWERMETER::current_queue.toArray()[int(i*step)]-mini_var)/(curve_scale*data_range);
            }
        }

        for(int i=0;i<190;i++){
            power_curve_point[i]+=curve_mini_height*135;
            voltage_curve_point[i]+=curve_mini_height*135;
            current_curve_point[i]+=curve_mini_height*135;
        }
        //绘制曲线
        for(int i=239;i>50;i--){
            clk.drawLine(i, 133-power_curve_point[239-i], i-1, 133-power_curve_point[240-i], TFT_BLUE);
            clk.drawLine(i, 133-voltage_curve_point[239-i], i-1, 133-voltage_curve_point[240-i], TFT_RED);
            clk.drawLine(i, 133-current_curve_point[239-i], i-1, 133-current_curve_point[240-i], TFT_GREEN);
        }

        clk.setTextColor(TFT_WHITE);
    }

    void wireless_page() {
        clk.setCursor(10, 15);
        clk.setTextFont(4);
        clk.setTextSize(1);
        clk.print("self MAC:");
        clk.setCursor(10, 40);
        char macStr[18];
        sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", self_Macaddress[0], self_Macaddress[1], self_Macaddress[2], self_Macaddress[3], self_Macaddress[4], self_Macaddress[5]);
        clk.print(macStr);
        clk.setCursor(10, 70);
        
        using NVSSTORAGE::pair_mac;
        if(pair_mac[0]==0xFF&&pair_mac[1]==0xFF&&pair_mac[2]==0xFF&&pair_mac[3]==0xFF&&pair_mac[4]==0xFF&&pair_mac[5]==0xFF){
            clk.setTextColor(TFT_RED);
            clk.print("broadcast mode");
        }else{
            clk.setTextColor(TFT_GREEN);
            clk.print("pair MAC:");
        }
        clk.setCursor(10, 95);
        
        sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", pair_mac[0], pair_mac[1], pair_mac[2], pair_mac[3], pair_mac[4], pair_mac[5]);
        clk.print(macStr);

        clk.setTextColor(TFT_WHITE);
    }
    // 页面列表
    std::list<std::function<void()>> page_list;                    // 存储多个页面
    std::list<std::function<void()>>::iterator now_page = page_list.begin(); // 当前页面

    // 屏幕更新任务
    void updatescreen(void * pvParameters) {
        tft.init();                                                // 初始化屏幕
        tft.setRotation(3);                                        // 设置屏幕方向
        tft.fillScreen(TFT_BLACK);                                 // 清空屏幕

        page_list.push_back(mian_page);                            // 添加主页面
        page_list.push_back(amperage_page);                        // 添加累计电量页面
        page_list.push_back(curve_page);                           // 添加曲线页面
        page_list.push_back(wireless_page);                        // 添加无线页面
        now_page = page_list.begin();                              // 初始化当前页面

        clk.createSprite(240, 135);                                // 创建缓冲区
        while (true) {
            tft.startWrite();
            clk.fillSprite(TFT_BLACK);                             // 清空缓冲区
            now_page->operator()();                                // 调用当前页面的绘制函数
            clk.drawRoundRect(0, 0, 240, 135, 0, TFT_YELLOW);      // 绘制矩形边框
            clk.pushSprite(0, 0);                                  // 将缓冲区内容推送到屏幕
            tft.endWrite();                                        // 结束写入
            delay(1000 / SCREEN_HZ);                               // 控制刷新率
        }
    }
}

#endif