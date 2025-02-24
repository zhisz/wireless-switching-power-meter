/*
 * @version: no version
 * @LastEditors: qingmeijiupiao
 * @Description: 网页控制相关代码
 * @author: qingmeijiupiao
 * @LastEditTime: 2025-02-24 12:38:00
 */
#ifndef WEB_HPP
#define WEB_HPP
#include <WiFi.h> //ESP32 Arduino SDK库
#include <DNSServer.h> //ESP32 Arduino SDK库
#include <WebServer.h> //ESP32 Arduino SDK库
#include <ArduinoJson.h>
#include "web/c_header/index_html.h"// 前端文件，这个头文件是自动生成的，源文件在include/web/src/
#include "static/HXCthread.hpp"
#include "static/POWERMETER.hpp"
#include "static/HXC_NVS.hpp"

namespace WEB{
    
//创建一个异步Web服务器
WebServer server(80);  

//后端线程
HXC::thread<void> web_thread([](){
    constexpr int web_feedback_hz = 100;// web后端轮询频率
    auto xLastWakeTime = xTaskGetTickCount();// 获取当前时间，用于控制刷新率
    while (true){
        server.handleClient(); // 处理web请求
        xTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/web_feedback_hz);// 等待
    }
});


//自动弹出界面实现线程
HXC::thread<void> DNS_thread([](){
    // 定义DNS服务器对象,用于自动弹出网页
    DNSServer dns;
    IPAddress apIP = WiFi.softAPIP();
    dns.start(53, "*", apIP);
    constexpr int DNS_feedback_hz = 10;// DNS轮询频率
    auto xLastWakeTime = xTaskGetTickCount();// 获取当前时间，用于控制刷新率
    while (true){
        dns.processNextRequest(); // 处理DNS请求
        xTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/DNS_feedback_hz);// 等待
    }
    
});

//web启动状态
static bool web_state=false;

bool get_web_state() {  
    return web_state;
}

//后端设置
void backend_server_setup(){
    if(web_state==true){
        return;
    }
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

    //创建 /data API 返回功率数据的json文件
    server.on("/data", [](){
        // 创建 JSON 文档
        JsonDocument doc;
        doc["voltage"] = POWERMETER::voltage;// 电压
        doc["current"] = POWERMETER::current;// 电流
        doc["mah"] = POWERMETER::output_mah;// 累计功耗
        doc["state"] = power_output.getstate();// 输出状态
        doc["time"] = millis();// 当前时间

        // 将 JSON 文档转换为字符串
        String json;
        serializeJson(doc, json);
        server.send(200, "application/json", json);// 发送 JSON 响应
    });

    //创建/ctrl API 控制输出
    server.on("/ctrl", [](){
        // 获取 POST 请求体中的数据
        String inputData = server.arg("plain");
        
        // 解析 JSON 数据
        JsonDocument doc;
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
    web_state=true;
};

//是否默认启动web
HXC::NVS_DATA<bool> is_default_start_wifi("start_wifi",false);

//wifi模式 true为AP模式 false为STA模式
HXC::NVS_DATA<bool> wifi_default_ap_mode("ap_mode",true);

//wifi名称
HXC::NVS_DATA<String> default_wifi_ssid("wifissid","HXC");

//wifi密码
HXC::NVS_DATA<String> default_wifi_password("wifipassword","");
/**
 * @brief : 网页初始化,调用该函数后，WIFI和web服务器都会启动
 * @return  {*}
 * @Author : qingmeijiupiao
 * @param {String} ssid :WIFI名称
 * @param {String} password :WIFI名称 无密码为  ""
 * @param {wifi_mode_t} mode :WIFI模式  WIFI_MODE_STA为连接wifi的模式  WIFI_MODE_AP为创建wifi热点的模式
 */
void setup(wifi_mode_t mode=WIFI_MODE_AP,String ssid="HXC",String password=""){
    if(web_state==true){
        return;
    }
    // 创建wifi访问点，设置名称和密码
    if(mode==WIFI_MODE_STA){ //STA模式
        WiFi.mode(mode);  // 设置为 STA 模式
        WiFi.begin(ssid.c_str(), password.c_str());
    }else{//AP模式
        WiFi.softAP(ssid.c_str(), password.c_str());
    }
    delay(100);
    //后端初始化
    backend_server_setup();
    delay(100);
    web_thread.start(/*taskname=*/"web",/*stacksize=*/8*1024);//web线程启动
    if(WiFi.getMode()!=WIFI_MODE_STA){//是AP模式
        delay(100);
        DNS_thread.start(/*taskname=*/"DNS",/*stacksize=*/2*1024);//DNS线程启动
    }
}; 

//web停止,断开wifi,该函数在测试中极不稳定，暂未启用
// void stop(){
//     server.close();
//     delay(100);
//     web_thread.stop();//web线程停止
//     delay(100);
//     if(WiFi.getMode()!=WIFI_MODE_STA){//是AP模式
//         DNS_thread.stop();//DNS线程停
//     }
//     web_state=false;//web状态关闭
//     if(WiFi.getMode()!=WIFI_MODE_STA){//是AP模式
//         WiFi.softAPdisconnect(true);//断开热点
//     }else{
//         WiFi.disconnect(true);//断开wifi
//     }
//     WiFi.mode(WIFI_OFF); 
// };
}

#endif