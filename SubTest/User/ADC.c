#include "ADC.h"

uint8_t BatAD_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_ADC1,ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode=DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //����ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //ADC �����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1; //˳����й���ת���� ADC ͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure); //����ָ���Ĳ�����ʼ������ ADCx
	ADC_Cmd(ADC1, ENABLE); //ʹ��ָ���� ADC1
	
	ADC_ResetCalibration(ADC1); //������λУ׼
	while(ADC_GetResetCalibrationStatus(ADC1)); //�ȴ���λУ׼����
	ADC_StartCalibration(ADC1); //���� AD У׼
	while(ADC_GetCalibrationStatus(ADC1)); //�ȴ�У׼����
	return 0;
}

u16 Get_ADVal(void)
{
	ADC_RegularChannelConfig(ADC1,ADC_Channel_8,1,ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)) ;
	return ADC_GetConversionValue(ADC1);
}

//��ȡ��ص�ѹ����λmV������10��ȡƽ��
u16 Get_BatVol(void)
{
	u32 Raw_Data=0;
	u8 i=0;
	for(;i<10;++i)
	{
		Raw_Data+=Get_ADVal();
	}
	return (Raw_Data*8965/3)>>12;//Raw_Data/10/4096*3.3*432/3912*1000
}


