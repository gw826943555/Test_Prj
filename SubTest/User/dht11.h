#ifndef __DHT11_H 
#define __DHT11_H  

#include "stm32f10x.h"
#ifdef __cplusplus
 extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////	  
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���; 
//Mini STM32������ 
//DHT11 ��������	    
//����ԭ��@ALIENTEK 
//������̳:www.openedv.com 
//�޸�����:2012/7/24  
//�汾��V1.0 
//��Ȩ���У�����ؾ��� 
//Copyright(C) ����ԭ�� 2009-2019 
//All rights reserved 
//////////////////////////////////////////////////////////////////////////////////	  
  
//IO�������� 
#define DHT11_IO_IN()  {GPIOB->CRL&=0XFFFFFF0F;GPIOB->CRL|=8<<4;} 
#define DHT11_IO_OUT() {GPIOB->CRL&=0XFFFFFF0F;GPIOB->CRL|=3<<4;} 
////IO��������	    
#define	DHT11_DQ_OUT (*((volatile uint32_t  *)(0x42218184))) //���ݶ˿�	PD3  
#define	DHT11_DQ_IN  (*((volatile uint32_t  *)(0x42218104)))  //���ݶ˿�	PD3

u8 DHT_Init(void);//��ʼ��DHT11 
void DHT_Start(void);
void DHT_TimerInit(void);
void DHT_StartTimer(uint16_t delayus);
u8 DHT_Decode(uint8_t *temp,uint8_t *humi);

#ifdef __cplusplus
}
#endif

#endif 

