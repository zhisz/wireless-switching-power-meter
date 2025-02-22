/*
 * @LastEditors: qingmeijiupiao
 * @Description: 串口命令行相关
 * @Author: qingmeijiupiao
 * @LastEditTime: 2025-02-22 22:01:17
 */
#ifndef SHELL_HPP
#define SHELL_HPP
#include "SimpleSerialShell.hpp"
#include "HXC_NVS.hpp"
#include "HXCthread.hpp"
#include "PowerCtrl.hpp"
extern HXC::NVS_DATA<uint16_t> esp_now_secret_key;
namespace SHELL{
    // 初始化shell
    void shell_init(void){
        shell.attach(Serial);


        /*↓↓↓↓↓↓↓↓在下面添加命令↓↓↓↓↓↓↓↓↓*/

        // info 打印信息
        shell.addCommand(F("info 显示编译信息"),[](int argc, char** argv){
            #ifdef IS_PRO_VERSION
            shell.println(F("Pro Version"));
            #else
            shell.println(F("Normal Version"));
            #endif
            shell.println(F( "Built " __DATE__));
            return 0;
        });


        
        // esp_now_secret_key 修改esp_now密钥
        shell.addCommand(F("set_esp_now_secret_key 设置esp_now密钥 [uint16_t]"),[](int argc, char** argv){
            if(argc!=2){
                shell.println(F("参数错误"));
                return -1;
            };

            uint16_t key=std::stoi(argv[1],nullptr,0);
            if(key<=0||key>0xFFFF){
                shell.println(F("参数错误"));
                shell.printf("你输入的:0x%X",key);
                return -1;
            }
            shell.println("修改成功");
            shell.printf("新密钥:0x%X",key);
            esp_now_secret_key=key;
            change_secret_key(esp_now_secret_key);
            return 0;
        });

        // get_esp_now_secret_key 获取esp_now密钥 
        shell.addCommand(F("get_esp_now_secret_key 获取esp_now密钥"),[](int argc, char** argv){
            shell.print("当前密钥:  ");
            uint16_t key=esp_now_secret_key.read();
            shell.printf("0x%X\n",key);
            return 0;
        });

        //printDATA 打印电压电流功率数据 
        shell.addCommand(F("printDATA 打印电压电流功率数据\n  printDATA [开关打印]"),[](int argc, char** argv){
            if(argc!=2){
                shell.println(F("参数错误"));
                return -1;
            };
            bool state=atoi(argv[1]);
            static HXC::thread<void> printDATA_thread([](){
                while(1){
                    Serial.print(PowerCtrl::power_data.voltage);
                    Serial.print(",");
                    Serial.println(PowerCtrl::power_data.current);
                    delay(10);
                }
            });
            if(state){
                PowerCtrl::ctrl_send_data(true,50);
                printDATA_thread.start();
            }else{
                printDATA_thread.stop();
                PowerCtrl::ctrl_send_data(false,50);
            }
            return 0;
        });

        //pair 配对
        shell.addCommand(F("pair 配对"),[](int argc, char** argv){
            PowerCtrl::send_pair_package();
            shell.println("配对请求已发送");
            delay(200);
            shell.println("配对MAC列表:");
            for (auto i=peer_mac_list.begin();i!=peer_mac_list.end();i++){
                shell.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",i->mac[0],i->mac[1],i->mac[2],i->mac[3],i->mac[4],i->mac[5]);
            }
            return 0;
        });
    }


    // shell线程
    HXC::thread<void> shell_thread([](){
        shell_init();
        while (1){
            shell.executeIfInput();
            delay(10);
        }
    });


}



#endif