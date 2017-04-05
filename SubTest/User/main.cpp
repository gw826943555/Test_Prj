#include "stm32f10x.h"
#include "Console.h"
#include "CCan.h"
#include "Timer.h"
#include "dht11.h"
#include "adc.h"

CanRxMsg CanRxBuf[10];
CCanRxMailbox _rxMailBox(CanRxBuf,10);

uint16_t IO_St=0x00;
uint16_t VoltData=0x00;
uint8_t  TempData=0x00;
uint8_t  HumiData=0x00;

void CommonConfig(void);
void IntBrdSend(void);
void IO_Init(void);
void IO_Read(void);
void Toggle_IO(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin_x);

int main(void)
{
	CommonConfig();
	BaseTimer::Instance()->initialize();
	
	CanRouter1.InitCanGpio(CCanRouter::GROUP_A11);
	CanRouter1.InitCan();
	_rxMailBox.attachToRouter(CanRouter1);
	_rxMailBox.setStdId(0x611);
	Timer TempCheck(500,500);
	Timer HeartBeat(500,2000);
	
	IO_Init();
	DHT_Init();
	DHT_TimerInit();
	BatAD_Init();//ADC_Init
	
	CanRxMsg RxTemp;
	while(1)
	{
		CanRouter1.runTransmitter();
		CanRouter1.runReceiver();
		Console::Instance()->runTransmitter();
		
		IO_Read();
		if(TempCheck.isAbsoluteTimeUp())
		{
			DHT_Start();
			VoltData=Get_BatVol();
			DHT_Decode(&TempData,&HumiData);
		}
		if(HeartBeat.isAbsoluteTimeUp())
		{
			Toggle_IO(GPIOA,GPIO_Pin_0);
			Toggle_IO(GPIOA,GPIO_Pin_1);
			Toggle_IO(GPIOA,GPIO_Pin_2);
			Toggle_IO(GPIOA,GPIO_Pin_3);
			Toggle_IO(GPIOA,GPIO_Pin_4);
			Toggle_IO(GPIOA,GPIO_Pin_5);
			Toggle_IO(GPIOA,GPIO_Pin_6);
			Toggle_IO(GPIOA,GPIO_Pin_7);
			Toggle_IO(GPIOA,GPIO_Pin_8);
			Toggle_IO(GPIOA,GPIO_Pin_9);
			Toggle_IO(GPIOA,GPIO_Pin_10);
			Toggle_IO(GPIOA,GPIO_Pin_11);
			Toggle_IO(GPIOB,GPIO_Pin_6);
			Toggle_IO(GPIOB,GPIO_Pin_7);
			Toggle_IO(GPIOB,GPIO_Pin_8);
			Toggle_IO(GPIOB,GPIO_Pin_9);
		}
		if(_rxMailBox.getMsg(&RxTemp)==true)
		{
			if(RxTemp.Data[0]==0x55)
			{
				IntBrdSend();
			}
		}
	}
}

void CommonConfig(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

void IntBrdSend(void)
{
	CanTxMsg TxTemp;
	TxTemp.StdId=0x610;
	TxTemp.IDE=CAN_Id_Standard;
	TxTemp.RTR=CAN_RTR_DATA;
	TxTemp.DLC=8;
	TxTemp.Data[0]=IO_St;
	TxTemp.Data[1]=IO_St>>8;
	TxTemp.Data[2]=TempData;
	TxTemp.Data[3]=HumiData;
	TxTemp.Data[4]=VoltData;
	TxTemp.Data[5]=VoltData>>8;
	CanRouter1.putMsg(TxTemp);
}

void IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|
															GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
															GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void IO_Read(void)
{
	IO_St=~GPIO_ReadInputData(GPIOB);
	IO_St=((IO_St>>2)&0x0f)+((IO_St>>6)&0x3F0);
}

void Toggle_IO(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin_x)
{
	if(GPIOx->ODR&GPIO_Pin_x)
	{
		GPIOx->ODR&=(~GPIO_Pin_x);
	}else{
		GPIOx->ODR|=GPIO_Pin_x;
	}
}




