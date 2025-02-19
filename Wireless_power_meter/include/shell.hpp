/*
 * @LastEditors: qingmeijiupiao
 * @Description: 串口命令行相关
 * @Author: qingmeijiupiao
 * @LastEditTime: 2025-02-19 17:48:46
 */
#ifndef SHELL_HPP
#define SHELL_HPP
#include "static/SimpleSerialShell.hpp"
#include "static/HXCthread.hpp"
#include "static/HXC_NVS.hpp"
#include "static/POWERMETER.hpp"
#include "OTHER_FUNCTION.hpp"
#include "web.hpp"
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

        // set_resistance 修改采样电阻配置
        shell.addCommand(F("set_resistance 设置采样电阻值 [单位mΩ]"),[](int argc, char** argv){
            if(argc!=2){
                shell.println(F("参数错误"));
                return -1;
            };
            float res=atof(argv[1]);
            if(res<=0||res>10){
                shell.println(F("参数错误"));
                return -1;
            }
            //赋值
            POWERMETER::sample_resistance=res;
            return 0;
        });

        // get_resistance 获取采样电阻配置
        shell.addCommand(F("get_resistance 获取采样电阻值"),[](int argc, char** argv){
            float res=POWERMETER::sample_resistance.read();
            shell.printf("\n当前设置采样电阻值: %fmΩ\n",res);
            return 0;
        });

        // esp_now_secret_key 修改esp_now密钥
        shell.addCommand(F("set_esp_now_secret_key 设置esp_now密钥 [uint16_t]"),[](int argc, char** argv){
            if(argc!=2){
                shell.println(F("参数错误"));
                return -1;
            };
            uint16_t key=atoi(argv[1]);
            if(key<=0||key>0xFFFF){
                shell.println(F("参数错误"));
                shell.printf("你输入的:0x%X",key);
                return -1;
            }
            shell.println("修改成功");
            shell.printf("新密钥:0x%X",key);
            WIRELESSCTRL::esp_now_secret_key=key;
            change_secret_key(WIRELESSCTRL::esp_now_secret_key);
            return 0;
        });

        // get_esp_now_secret_key 获取esp_now密钥 
        shell.addCommand(F("get_esp_now_secret_key 获取esp_now密钥"),[](int argc, char** argv){
            shell.print("当前密钥:  ");
            uint16_t key=WIRELESSCTRL::esp_now_secret_key.read();
            shell.printf("0x%X\n",key);
            return 0;
        });

        // get_protect_state 获取保护状态
        shell.addCommand(F("get_protect_state 获取保护状态"),[](int argc, char** argv){
            shell.print("保护状态:  ");
            shell.printf("电流保护:%s\n",OTHER_FUNCTION::default_current_protect_state.read()?"开":"关");
            shell.printf("电压保护:%s\n",OTHER_FUNCTION::default_low_voltage_protect_state.read()?"开":"关");
            shell.printf("高温保护:%s\n",OTHER_FUNCTION::default_high_temperature_protect_state.read()?"开":"关");
            return 0;
        });

        // set_voltage_protect 设置电压保护
        shell.addCommand(F("set_voltage_protect 设置电压保护\n  set_voltage_protect [默认是否开启] [低压值:V]"),[](int argc, char** argv){
            if(argc!=3){
                shell.println(F("参数错误"));
                return -1;
            };
            bool state=atoi(argv[1]);
            float value=atof(argv[2]);
            if(value<=0||value>1000)
            {
                shell.println(F("参数错误"));
                return -1;
            }
            OTHER_FUNCTION::default_low_voltage_protect_state=state;
            OTHER_FUNCTION::low_voltage_protect_value=value;
            OTHER_FUNCTION::voltage_protect_ctrl(state);
            return 0;
        });
        // set_current_protect 设置电流保护
        shell.addCommand(F("set_current_protect 设置电流保护\n  set_current_protect [默认是否开启] [电流值:A]"),[](int argc, char** argv){
            if(argc!=3){
                shell.println(F("参数错误"));
                return -1;
            };
            bool state=atoi(argv[1]);
            float value=atof(argv[2]);
            if(value<=0||value>300)
            {
                shell.println(F("参数错误"));
                return -1;
            }
            OTHER_FUNCTION::default_current_protect_state=state;
            OTHER_FUNCTION::current_protect_value=value;
            OTHER_FUNCTION::current_protect_ctrl(state);
            return 0;
        });

        // set_high_temperature_protect 设置高温保护
        shell.addCommand(F("set_high_temperature_protect 设置高温保护\n  set_high_temperature_protect [默认是否开启] [高温值:℃]"),[](int argc, char** argv){
            if(argc!=3){
                shell.println(F("参数错误"));
                return -1;
            };
            bool state=atoi(argv[1]);
            float value=atof(argv[2]);
            if(value<=0||value>150){
                shell.println(F("参数错误"));
                return -1;
            }
            OTHER_FUNCTION::default_high_temperature_protect_state=state;
            OTHER_FUNCTION::high_temperature_protect_value=value;
            OTHER_FUNCTION::temperature_protect_ctrl(state);
            return 0;
        });
        //printDATA 打印电压电流功率数据 
        shell.addCommand(F("printDATA 打印电压电流功率数据\n  printDATA [开关打印]"),[](int argc, char** argv){
            if(argc!=2){
                shell.println(F("参数错误"));
                return -1;
            };
            bool state=atoi(argv[1]);
            OTHER_FUNCTION::serial_print_ctrl(state);
            return 0;
        });

        //wifi 开启网页功能
        shell.addCommand(F("wifi [开关wifi服务] [是AP模式] [wifi名称] [wifi密码 null为无密码 ]"),[](int argc, char** argv){
            if(argc<2){
                shell.println(F("参数错误\n wifi [开关wifi服务] [是AP模式] [wifi名称] [wifi密码 null为无密码 ]"));
                return -1;
            };
            
            //设置是否开启wifi和web
            WEB::is_default_start_wifi=atoi(argv[1]);
            
            if(argc>=3){
                //设置wifi模式 1为AP模式 0为STA模式
                WEB::wifi_default_ap_mode=atoi(argv[2]);
            }
            if(argc>=4){
                //设置wifi名称
                String ssid=argv[3];
                shell.printf("wifi名称:%s\n",ssid.c_str());
                WEB::default_wifi_ssid=ssid;
            }
            
            if(argc>=5){
                //设置wifi密码
                String password=argv[4];
                if(password=="null"){
                    password="";
                }
                shell.printf("wifi密码:%s\n",password.c_str());
                WEB::default_wifi_password=password;
            }
            shell.println(F("wifi 状态设置成功 马上重启"));
            delay(1000);
            ESP.restart();
            return 0;
        });

        //get_ip
        shell.addCommand(F("get_ip 获取ip"),[](int argc, char** argv){
            String ip="error";
            if(WiFi.getMode()!=WIFI_MODE_STA){//是AP模式
                shell.println("AP模式");
                shell.printf("AP ssid:%s\n",WiFi.softAPSSID().c_str());
                ip=WiFi.softAPIP().toString();
            }else{
                shell.println(F("STA模式"));
                ip=WiFi.localIP().toString();
            }

            shell.println(ip);
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