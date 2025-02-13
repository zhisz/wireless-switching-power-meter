#ifndef WEB_HPP
#define WEB_HPP

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ArduinoJson.hpp>
#include "web/c_header/index_html.h"
#include "static/HXCthread.hpp"
//创建一个异步Web服务器
WebServer server(80);  

// 定义DNS服务器对象,用于自动弹出网页
DNSServer dns;

void handleRoot() {
    // 发送网页内容
      server.send(200, "text/html", index_html);
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

HXC::thread<void> web_thread([](){
    while (true){
        dns.processNextRequest(); // 处理DNS请求
        server.handleClient(); // 处理web请求
        delay(10);
    }
});
void web_setup(){
    // 启动DNS服务器，将所有域名解析到wifi访问点的IP地址，实现重定向功能
    // 创建wifi访问点，设置名称和密码
    WiFi.softAP(ssid, password);
    // 获取wifi访问点的IP地址
    IPAddress apIP = WiFi.softAPIP();
    dns.start(53, "*", apIP);
    server.on("/", handleRoot); // 根目录请求
    server.on("/gen_204", handleDetect); // Google设备检测请求
    server.on("/hotspot-detect.html", handleDetect); // Apple设备检测请求
    server.onNotFound(handleNotFound); // 其他不存在的请求
    server.begin();
    web_thread.start(/*taskname=*/"web",/*stacksize=*/4096);//web线程启动
};


#endif