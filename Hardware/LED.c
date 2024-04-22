#include "stm32f10x.h"                  // Device header

/**
  * 函    数：LED初始化
  * 参    数：无
  * 返 回 值：无
  */
void LED_Init(void)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);        //开启GPIOC的时钟
    
    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);                        //将PA1和PA2引脚初始化为推挽输出
    
    /*设置GPIO初始化后的默认电平*/
    GPIO_SetBits(GPIOC, GPIO_Pin_14);                //设置PA1和PA2引脚为高电平
}

/**
  * 函    数：LED1开启
  * 参    数：无
  * 返 回 值：无
  */
void LED1_ON(void)
{
    GPIO_SetBits(GPIOC, GPIO_Pin_14);        //设置PC14引脚为高电平
}

/**
  * 函    数：LED1关闭
  * 参    数：无
  * 返 回 值：无
  */
void LED1_OFF(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_14);        //设置PC14引脚为低电平
}

/**
  * 函    数：LED1状态翻转
  * 参    数：无
  * 返 回 值：无
  */
void LED1_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_14) == 0)        //获取输出寄存器的状态，如果当前引脚输出低电平
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_14);                    //则设置PC14引脚为高电平
    }
    else                                                    //否则，即当前引脚输出高电平
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_14);                    //则设置PC14引脚为低电平
    }
}

///**
//  * 函    数：LED2开启
//  * 参    数：无
//  * 返 回 值：无
//  */
//void LED2_ON(void)
//{
//    GPIO_SetBits(GPIOC, GPIO_Pin_15);        //设置PC15引脚为高电平
//}

///**
//  * 函    数：LED2关闭
//  * 参    数：无
//  * 返 回 值：无
//  */
//void LED2_OFF(void)
//{
//    GPIO_ResetBits(GPIOC, GPIO_Pin_15);        //设置PC15引脚为低电平
//}

///**
//  * 函    数：LED2状态翻转
//  * 参    数：无
//  * 返 回 值：无
//  */
//void LED2_Turn(void)
//{
//    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_15) == 0)        //获取输出寄存器的状态，如果当前引脚输出低电平
//    {                                                  
//        GPIO_SetBits(GPIOC, GPIO_Pin_15);                       //则设置PC15引脚为高电平
//    }                                                  
//    else                                                       //否则，即当前引脚输出高电平
//    {                                                  
//        GPIO_ResetBits(GPIOC, GPIO_Pin_15);                     //则设置PC15引脚为低电平
//    }
//}
