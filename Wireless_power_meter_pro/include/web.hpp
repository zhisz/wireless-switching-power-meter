/*
 * @version: no version
 * @LastEditors: qingmeijiupiao
 * @Description: 网页控制相关代码
 * @author: qingmeijiupiao
 * @LastEditTime: 2025-02-14 11:51:28
 */
#ifndef WEB_HPP
#define WEB_HPP
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "web/c_header/index_html.h"
#include "static/HXCthread.hpp"
#include "static/POWERMETER.hpp"
//创建一个异步Web服务器
WebServer server(80);  

HXC::thread<void> web_thread([](){
    constexpr int web_feedback_hz = 100;
    auto xLastWakeTime = xTaskGetTickCount();// 获取当前时间，用于控制刷新率
    while (true){
        server.handleClient(); // 处理web请求
        xTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/web_feedback_hz);// 等待
    }
});


//自动弹出界面实现
HXC::thread<void> DNS_thread([](){
    // 定义DNS服务器对象,用于自动弹出网页
    DNSServer dns;
    IPAddress apIP = WiFi.softAPIP();
    dns.start(53, "*", apIP);
    constexpr int DNS_feedback_hz = 10;
    auto xLastWakeTime = xTaskGetTickCount();// 获取当前时间，用于控制刷新率
    while (true){
        dns.processNextRequest(); // 处理DNS请求
        xTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/DNS_feedback_hz);// 等待
    }
    
});


//后端设置
void server_setup(){
    // 根目录请求
    server.on("/", [](){
        // 发送网页内容
        server.send_P(200, "text/html", index_html);
    });
    // 处理设备检测的请求
    auto handleDetect=[](){
        // 发送一个空白的响应，表示已经连接到网络，不需要登录
        server.send(204);
    };
    // Google设备检测请求
    server.on("/gen_204", handleDetect); 
    // Apple设备检测请求
    server.on("/hotspot-detect.html", handleDetect);

    //创建/data API 返回功率数据的json文件
    server.on("/data", [](){
        float voltage = POWERMETER::voltage;
        float current = POWERMETER::current;
        float power = voltage * current;
        float mah = POWERMETER::output_mah;
        float mwh = POWERMETER::output_mwh;
    
        JsonDocument doc;
        doc["voltage"] = voltage;
        doc["current"] = current;
        doc["power"] = power;
        doc["mah"] = mah;
        doc["state"] = power_output.getstate();
        String json;
        serializeJson(doc, json);
        server.send(200, "application/json", json);
    });
    //创建/ctrl API 控制输出
    server.on("/ctrl", [](){
        // 获取 POST 请求体中的数据
        String inputData = server.arg("plain");
        
        // 解析 JSON 数据
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, inputData);
        
        if (error) {
            Serial.println("JSON 解析失败");
            server.send(400, "text/plain", "JSON 解析失败");
            return;
        }
        
        // 获取开关状态
        bool state = doc["state"];
        if(state){
            power_output.off();
        }else{
            power_output.on();
        }
        
        // 返回成功响应
        server.send(200, "text/plain", "OK");
        
    });
    // 其他不存在的请求
    server.onNotFound([](){
        // 重定向到根目录，弹出网页
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });
    // 启动服务器
    server.begin();
};

//网页初始化
void web_setup(String ssid="HXC",String password="",wifi_mode_t mode=WIFI_AP){
    // 创建wifi访问点，设置名称和密码
    if(mode==WIFI_STA){
        WiFi.mode(mode);  // 设置为 STA 模式
        WiFi.begin(ssid.c_str(), password.c_str());
    }else{
        WiFi.softAP(ssid.c_str(), password.c_str());
    }
    //后端设置
    server_setup();

    web_thread.start(/*taskname=*/"web",/*stacksize=*/4096);//web线程启动
    if(WiFi.getMode()!=WIFI_STA){
        DNS_thread.start(/*taskname=*/"DNS",/*stacksize=*/2048);//DNS线程启动
    }
};


#endif