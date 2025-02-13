#ifndef WEB_HPP
#define WEB_HPP

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "web/c_header/index_html.h"
#include "web/c_header/jquery_3_7_1_min_js.h"
#include "web/c_header/highcharts_js.h"
#include "static/HXCthread.hpp"
#include "static/POWERMETER.hpp"
//创建一个异步Web服务器
WebServer server(80);  

// 定义DNS服务器对象,用于自动弹出网页
DNSServer dns;

void handleRoot() {
    // 发送网页内容
    server.send_P(200, "text/html", index_html);
}


// 处理设备检测的请求
void handleDetect() {
    // 发送一个空白的响应，表示已经连接到网络，不需要登录
    server.send(204);
}

// 处理不存在的请求
void handleNotFound() {
    // 重定向到根目录，弹出网页
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

//创建/data API 返回功率数据的json文件
void handleData() {
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
}

HXC::thread<void> web_thread([](){
    constexpr int web_feedback_hz = 10;
    auto xLastWakeTime = xTaskGetTickCount();// 获取当前时间，用于控制刷新率
    while (true){
        //dns.processNextRequest(); // 处理DNS请求
        server.handleClient(); // 处理web请求
        xTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ/web_feedback_hz);// 等待
    }
});

void web_setup(){
    // 创建wifi访问点，设置名称和密码
    WiFi.mode(WIFI_STA);  // 设置为 STA 模式
    WiFi.begin("helloiip", "20210928MYH");
    //等联网
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    // // 获取wifi访问点的IP地址
    // IPAddress apIP = WiFi.softAPIP();
    // dns.start(53, "*", apIP);

    server.on("/", handleRoot); // 根目录请求
    server.on("/gen_204", handleDetect); // Google设备检测请求
    server.on("/hotspot-detect.html", handleDetect); // Apple设备检测请求
    server.on("/data", handleData);
    server.on("/jquery-3.7.1.min.js", [](){
        server.send_P(200, "text/javascript", jquery_3_7_1_min_js);
        
    });
    server.on("/highcharts.js", [](){
        server.send_P(200, "text/javascript", highcharts_js);
    });
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
            power_output.on();
        }else{
            power_output.off();
        }
        
        // 返回成功响应
        server.send(200, "text/plain", "OK");
        
    });
    server.onNotFound(handleNotFound); // 其他不存在的请求
    
    server.begin();
    web_thread.start(/*taskname=*/"web",/*stacksize=*/4096);//web线程启动
};


#endif