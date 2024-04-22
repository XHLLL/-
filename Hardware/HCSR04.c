#include "stm32f10x.h"                  // Device header
#include "Delay.h"

#define Echo GPIO_Pin_11        //HC-SR04模块的Echo脚接GPIOB11
#define Trig GPIO_Pin_12        //HC-SR04模块的Trig脚接GPIOB12

int ultasonic=0;
float dis=0;
int count=0;

void HC_SR04_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        //设置GPIO口为推挽输出
    GPIO_InitStructure.GPIO_Pin = Trig;                        //设置GPIO口12
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        //设置GPIO口速度50Mhz
    GPIO_Init(GPIOA,&GPIO_InitStructure);                    //初始化GPIOA

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;            //设置GPIO口为下拉输入模式
    GPIO_InitStructure.GPIO_Pin = Echo;                        //设置GPIO口11
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);                    //初始化GPIOA

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period=49999;
    TIM_TimeBaseInitStruct.TIM_Prescaler=72;
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
    
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel=TIM4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
    NVIC_Init(&NVIC_InitStruct);
    TIM_Cmd(TIM4,ENABLE);
}

void TIM4_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM4,TIM_FLAG_Update)==1)//使能中断标志位
    {
        ultasonic+=1;
        TIM_ClearITPendingBit(TIM4,TIM_FLAG_Update);//清除中断标志位
    }
    
}

float ultasonic_Sign(void)
{
    GPIO_ResetBits(GPIOA,Trig);//拉低
    GPIO_SetBits(GPIOA,Trig);//拉高
    Delay_us(50);
    GPIO_ResetBits(GPIOA,Trig);//拉低，驱动超声波模块
    while(GPIO_ReadInputDataBit(GPIOA,Echo)==0);//等待信号发出，信号发出，计数器置0，信号发出，变为高电平，引脚置1
//    ultasonic=0;
    TIM4->CNT=0;//us级别读取
    while(GPIO_ReadInputDataBit(GPIOA,Echo)==1);//等待信号接收，信号发出过程中，引脚一直置位1
    count=ultasonic;
        count=TIM4->CNT;
    dis=(float)count/58;
        
    return dis;
        
      
}
