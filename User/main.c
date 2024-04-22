#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Buzzer.h"
#include "Key.h"
#include "LED.h"
#include "Timer.h"
#include "CountSensor.h"
#include "DS18B20.h"
#include "Motor.h"
#include "AT24C02.h"
//#include "HCSR04.h"
#include "stdio.h"


#define EEPROM_MILEAGE_ADDR 0x00    //数据存储地址

uint8_t KeyNum;
int8_t motor_speed;
int32_t last_CountSensor_Count = 0; 
int32_t last_second_CountSensor_Count = 0; // 保存上一秒的计数值
uint8_t speed_max = 30;//速度报警值
int _1s_number = 0;
float speed = 0.0;
float mileage = 0.0;
char display_buf[32];   //显示缓存区
char tempStr[10];
short temperature;
//float distance;
float pi = 3.1415926;   //圆周率
float Diameter = 26.0; //车轮直径，单位为英寸
extern uint32_t CountSensor_Count;

/*********************************************
 * 创建一个结构体
 * 存放界面标志位
 */
typedef struct
{
    u8 Interface_Mark;     //界面状态
} Mark;
Mark Mark_Sign;//状态标志位

/*********************************************
 * 创建一个枚举
 * 存放界面变量
 */
enum
{
    Main_Interface = 0x10, /****主界面*****/
    Measure_Interface = 0x20, /****测量界面***/
    Set_Interface = 0x30, /****设置报警值界面***/
    SetDiameter_Interface = 0x40, /****选择车轮尺寸界面*****/
};


void DS18B20_GET(void);
void HCSR04_GET(void);
void Key_function(void);
void Display_function(void);
void Warn_function(void);

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//配置NVIC为分组2

    /*模块初始化*/
    OLED_Init();            //OLED初始化
    DS18B20_Init();         //温度传感器初始化
    CountSensor_Init();     //计数传感器初始化
    Motor_Init();           //电机驱动初始化
    Timer_Init();           //定时器初始化
    Key_Init();             //按键初始化
    LED_Init();             //LED初始化
    Buzzer_Init();          //蜂鸣器初始化
    AT24C02_Init();         //数据存储芯片初始化
//    HC_SR04_Init();

    AT24C02_Read_Float(EEPROM_MILEAGE_ADDR, &mileage, 1);

    OLED_Clear();
    while (1)
    {
        DS18B20_GET();
//        HCSR04_GET();
        Key_function();
        Display_function();
        Warn_function();
        DS18B20_Start();
        DS18B20_Get_Temp();
            // 只有在计数器正在增加时才更新里程
            if (CountSensor_Count > last_CountSensor_Count)
            {

                // 累加所有时间的计数，用于计算里程
                mileage += ((float)(CountSensor_Count - last_CountSensor_Count) / 20.0) * (Diameter * 0.0254) * pi / 1000;
                // 每次更新里程后都写入EEPROM
                AT24C02_Write_Float(EEPROM_MILEAGE_ADDR, &mileage);
                last_CountSensor_Count = CountSensor_Count;

            }


        //Delay_ms(100);
    }

}


//void HCSR04_GET(void)
//{
//     distance = ultasonic_Sign();
//}

/*按键函数*/
void Key_function(void)
{
    KeyNum = Key_GetNum();

    if (KeyNum != 0)
    {
        switch(KeyNum)
        {
            case 1:     //按键1，主界面
                Mark_Sign.Interface_Mark = Main_Interface;
                OLED_Clear();
            break;

            case 2:     //按键2，切换界面
                if(Main_Interface == Mark_Sign.Interface_Mark)
                {
                    Mark_Sign.Interface_Mark = Measure_Interface;
                    OLED_Clear();
                }
                else if(Measure_Interface == Mark_Sign.Interface_Mark)
                {
                    Mark_Sign.Interface_Mark = Set_Interface;
                    OLED_Clear();
                }
                else if(Set_Interface == Mark_Sign.Interface_Mark)
                {
                    Mark_Sign.Interface_Mark = SetDiameter_Interface;
                    OLED_Clear();
                }
                else{
                    Mark_Sign.Interface_Mark = Measure_Interface;
                    OLED_Clear();
                }
            break;

            case 3:     /*按键3，测量界面时按下加速电机转速，
                          设置报警值界面按下增加报警值，选择
                          车轮尺寸界面按下切换车轮尺寸，有26、
                          27.5、29寸选择*/
                switch (Mark_Sign.Interface_Mark)
                {
                    case Measure_Interface:     //测量界面

                        motor_speed += 10;
                        if(motor_speed > 100)
                            motor_speed = 0;
                        Motor_SetSpeed(motor_speed);
                    break;

                    case Set_Interface:         //设置报警值界面
                        if(speed_max < 99)
                            speed_max++;
                    break;

                    case SetDiameter_Interface: //车轮尺寸界面
                        if(Diameter == 26.0)
                            Diameter = 27.5;
                        else if(Diameter == 27.5)
                            Diameter = 29.0;
                        else Diameter = 26.0;
                    break;

                    default:
                    break;
                }
            break;

            case 4:     /*按键4，测量界面时按下减速电机转速，
                          设置报警值界面按下减少报警值*/
                switch(Mark_Sign.Interface_Mark)
                {
                    case Measure_Interface:     //测量界面

                        motor_speed -= 20;
                        if(motor_speed < 0)
                            motor_speed = 0;
                        Motor_SetSpeed(motor_speed);
                    break;

                    case Set_Interface:         //设置报警值界面
                        if(speed_max > 0)
                            speed_max--;
                    break;

                    default:
                    break;
                }
            break;

            case 5:     //按键5，按下清零里程存储值
                switch(Mark_Sign.Interface_Mark)
                {
                    case Measure_Interface:
                        // 擦除存储的里程数据
                        AT24C02_Erase(EEPROM_MILEAGE_ADDR, sizeof(float));
                        // 清零里程值
                        mileage = 0.0;
                }
            break;

        default:
        break;
        }
    }
}

/*显示函数*/
void Display_function(void)
{
    switch(Mark_Sign.Interface_Mark)        //根据不同的显示模式标志位，显示不同的界面
    {
        case Main_Interface:         //界面0：测量界面，显示测量值
            OLED_ShowChinese(32, 24, "超速报警");
            //OLED_ShowString(16,24,"Speed Alarm",OLED_8X16);
            OLED_Update();
        break;
    
        case Measure_Interface:
//            OLED_ShowString(0, 0, "SetSpeed:",OLED_8X16);
//            OLED_ShowSignedNum(75, 0, motor_speed, 3,OLED_8X16);
//            OLED_ShowChinese(0, 0, "距离：");
//            sprintf(display_buf, "%.2fcm   ", distance);
//            OLED_ShowString(40, 0, display_buf, OLED_8X16);
            
            //OLED_ShowString(0,0,"Mileage:",OLED_8X16);
            OLED_ShowChinese(0, 0,"里程：");
            sprintf(display_buf, "%.2fkm", mileage);
            OLED_ShowString(40, 0, display_buf,OLED_8X16);
            
            //OLED_ShowString(0,24,"Speed:",OLED_8X16);
            OLED_ShowChinese(0, 24,"速度：");
            sprintf(display_buf, "%.2fkm/h    ", speed);
            OLED_ShowString(40, 24, display_buf,OLED_8X16);
            
            //OLED_ShowString(0,48,"Temp:",OLED_8X16);
            OLED_ShowChinese(0, 48,"温度：");
            sprintf(tempStr, "%+d.%d", temperature / 10, temperature % 10);// 将温度转换为字符串 temperature / 10为整数部分，temperature % 10为小数部分
            OLED_ShowString(40, 48, tempStr,OLED_8X16);
            //OLED_ShowString(80,48,"C",OLED_8X16);
            OLED_ShowChinese(80, 48,"℃");
            OLED_Update();
        break;

        case Set_Interface:
            //OLED_ShowString(0,16,"Set warning vlaue",OLED_6X8);
            OLED_ShowChinese(8, 16,"设置速度报警值");
            sprintf(display_buf,"%d",speed_max);
            OLED_ShowString(56, 32, display_buf,OLED_8X16);
            OLED_Update();
        break;

        case SetDiameter_Interface:
            //OLED_ShowString(0,16,"Set Diameter Of Wheels",OLED_6X8);
            OLED_ShowChinese(16, 16,"设置车轮尺寸");
            sprintf(display_buf,"%.1f",Diameter);
            OLED_ShowString(48, 32, display_buf,OLED_8X16);
            OLED_Update();
        break;

        default:
        break;
    }
    OLED_Update();
}

/*速度测量函数*/
void TIM3_IRQHandler(void)
{

        if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
        {
            _1s_number++;

            if (_1s_number == 100) // 每秒更新一次速度
            {   
                //计算公式：速度=（上一秒脉冲计数/一圈的的脉冲技术20） * 轮胎直径*0.0254（转化单位为米） *圆周率 *3.6（转化为km/h）
                speed = ((float)(CountSensor_Count - last_second_CountSensor_Count) / 20) * (Diameter * 0.0254) * pi * 3.6; // 计算速度，使用上一秒的计数值
                last_second_CountSensor_Count = CountSensor_Count; // 更新上一秒的计数值
                _1s_number = 0;
            }

            TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    }
}

/*报警函数，当测量速度大于设定报警值时进行声光报警*/
void Warn_function(void)
{
    if(Mark_Sign.Interface_Mark == Measure_Interface)
    {
        if(speed > speed_max)
        {
            Buzzer_ON();Delay_ms(50);
            LED1_ON();Delay_ms(50);
            Buzzer_OFF();Delay_ms(50);
            LED1_OFF();Delay_ms(50);
            Buzzer_ON();Delay_ms(50);
            LED1_ON();Delay_ms(50);
            Buzzer_OFF();Delay_ms(50);
            LED1_OFF();Delay_ms(50);
        }
//        if(distance < 100)
//        {
//            //Buzzer_ON();Delay_ms(100);
//            LED2_ON();Delay_ms(100);
//            //Buzzer_OFF();Delay_ms(100);
//            LED2_OFF();Delay_ms(100);
//            //Buzzer_ON();Delay_ms(100);
//            LED2_ON();Delay_ms(100);
//            //Buzzer_OFF();Delay_ms(100);
//            LED2_OFF();Delay_ms(100);
//        }
        else
        {
            Buzzer_OFF();
            LED1_OFF();
//            LED2_OFF();
        }
    }
    else
    {
        motor_speed = 0;
        CountSensor_Count = 0;
        Buzzer_OFF();
        LED1_OFF();
        //LED2_OFF();
    }
}

/*获取温度函数*/
void DS18B20_GET(void)
{
    temperature = DS18B20_Get_Temp(); // 读取温度
    Delay_ms(500);
}

