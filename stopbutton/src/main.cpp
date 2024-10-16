/*
 * @LastEditors: qingmeijiupiao
 * @Description
 * @Author: qingmeijiupiao
 * @Date: 2024-09-14 21:27:28
 */
#include <Arduino.h>
#include <U8g2lib.h>
#include "PowerCtrl.hpp"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE,/*clk*/5,/*data*/4);
                        // 设备名，制造商，电量
const int buttonPIN = 0;    // 定义按键端口
const int LEDPIN = 3;    // 定义LED端口
const int buzzer = 10;    // 定义蜂鸣器端口
const int volt_read_pin=1;    // 定义电池电压采样端口

namespace Battery{
  float battery_voltage = 0;    // 电池电压
  float quantity = 0;    // 电池电量
  const float mini_voltage = 3.3;    // 电池最低电压
  void voltreadtask(void *pvParameters){
    while (true){
      battery_voltage = 2*analogReadMilliVolts(volt_read_pin)/1000.0;
      quantity = (battery_voltage-mini_voltage)/4.2-mini_voltage;
      if(quantity<0){
        quantity = 0;
      }
      if(quantity>1){
        quantity = 1;
      }
      delay(1000);
    }
  }
}



void screentask( void *pvParameters ) {
  while (true){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(4,30,"RUNING!");
    u8g2.setFont(u8g2_font_courB08_tf);
    u8g2.setCursor(0,50);
    u8g2.print(Battery::battery_voltage);
    u8g2.sendBuffer(); 
    delay(100);
  }

}
void LEDTask(void* p){
    pinMode(LEDPIN, OUTPUT);
    ledcAttachPin(LEDPIN, 0);
    ledcSetup(0, 5000, 8);
    while(1){
      for(int i=0;i<255;i++){
        ledcWrite(0,i);
        delay(5);
      }
      for(int i=0;i<255;i++){
        ledcWrite(0,255-i);
        delay(5);
      }
    }
}

bool detect_usb_plugin() {//检测是否插入USB
  int FIFO=Serial.availableForWrite();//记录串口缓冲区剩余空间
  Serial.println("detect_usb_plugin");//串口打印一些信息
  delay(10);//等待发送
  int FIFO2=Serial.availableForWrite();//再次记录串口缓冲区剩余空间
  if(FIFO==FIFO2){//如果两次记录的剩余空间相同，说明数据发送出去了，插入USB
    return true;
  }else{
    return false;//如果两次记录的剩余空间不同，说明没有插入USB
  }
}

void setup() 
{
  u8g2.begin();
  Serial.begin(115200);
  pinMode(buttonPIN, INPUT_PULLUP);
  xTaskCreate(screentask, "screentask", 2048, NULL, 5, NULL);
  xTaskCreate(LEDTask, "LEDTask", 512, NULL, 5, NULL);
  xTaskCreate(Battery::voltreadtask, "voltreadtask", 512, NULL, 1, NULL);
  PowerCtrl::setup();

}
 
void loop() {}
