#include "dht11.h" 

uint16_t DHT_CapBuf[45];
uint8_t i=0;

//��ʼ��DHT11��IO�� DQ ͬʱ���DHT11�Ĵ��� 
//����1:������ 
//����0:����    	  
u8 DHT_Init(void) 
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIOB->CRL&=0XFFFFFF0F;//PORTB.1 ������� 
	GPIOB->CRL|=0X00000070; 
	GPIOB->ODR|=1<<1;      //���1	   
	return 0;  
}

void DHT_TimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=71;
	TIM_TimeBaseInitStructure.TIM_Period=50000;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM3,DISABLE);
	
	TIM_ICInitTypeDef TIM3_ICInitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;
	
	TIM3_ICInitStructure.TIM_Channel=TIM_Channel_4;
	TIM3_ICInitStructure.TIM_ICFilter=0x00;
	TIM3_ICInitStructure.TIM_ICPolarity=TIM_ICPolarity_Rising;
	TIM3_ICInitStructure.TIM_ICSelection=TIM_ICSelection_DirectTI;
	TIM3_ICInitStructure.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	TIM_ICInit(TIM3,&TIM3_ICInitStructure);
}

void DHT_StartTimer(uint16_t delayus)
{
	TIM_SetCounter(TIM3,0);
	TIM_Cmd(TIM3,ENABLE);
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);					//���TIM3�жϹ����־λ���ȴ���ʱ����
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_CC4,DISABLE);
}

void DHT_Start(void)
{
//	if(DHT_CapBuf[43]==0xaa)
//		return ;																				//��һ֡û�н���
	DHT11_DQ_OUT=1;
	DHT11_IO_OUT();
	DHT11_DQ_OUT=0;
	DHT_StartTimer(50000);
}

u8 DHT_Decode(uint8_t *temp,uint8_t *humi)
{
	uint64_t dat=0;
//	if(DHT_CapBuf[43]!=0xaa)
//		return 1;																			//����δ�������
	if(DHT_CapBuf[1]>130&DHT_CapBuf[1]<190)
	{
		for(uint8_t i=2;i<42;++i)
		{
			dat<<=1;
			if(DHT_CapBuf[i]>110&DHT_CapBuf[i]<130)
			{
				dat+=1;
			}else if(DHT_CapBuf[i]>70&DHT_CapBuf[i]<90)
			{
			}else
			{
				return 1;																	//���ݴ���
			}
		}
		*humi=dat>>32;
		*temp=(dat>>16)&0xFf;
		if((*temp+*humi)==(dat&0xFf))
		{
			DHT_CapBuf[43]=0;
			return 0;																	//���ݽ������
		}
		else
			return 1;																	//����У�����
	}
	return 1;																			//��ʼ֡���ȴ���
}
#ifdef __cplusplus
 extern "C" {
#endif
	 
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{
		i=0;
		DHT11_DQ_OUT=1;
		DHT11_IO_IN();
		TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);
		TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);
		TIM_SetCounter(TIM3,0);
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
		TIM_ClearITPendingBit(TIM3,TIM_IT_CC4);					//������벶��ͨ��4�жϹ����־λ
		DHT_CapBuf[43]=0;
	}
	if(TIM_GetITStatus(TIM3,TIM_IT_CC4)==SET)
	{
		DHT_CapBuf[i]=TIM_GetCapture4(TIM3);
		TIM_SetCounter(TIM3,0);
		++i;
		if(i>42)
		{
			TIM_ITConfig(TIM3,TIM_IT_CC4,DISABLE);
			DHT11_IO_OUT();
			DHT11_DQ_OUT=1;
			//DHT_CapBuf[i]=0xaa;
		}
		TIM_ClearITPendingBit(TIM3,TIM_IT_CC4);
	}
}

#ifdef __cplusplus
 }
 
#endif 

