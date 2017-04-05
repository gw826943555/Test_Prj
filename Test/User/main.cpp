#include "stm32f4xx.h"
#include "CUsart.h"
#include "Timer.h"
#include "CCan.h"

#define UID (*(__IO u32*)(0x1FFF7A10))

//////////////////////////////////////////////////////////////////////////////
void RS485_CommunicationProtcol(uint8_t* buf);
int RS485_Connect(void);
void CAN1_CommunicationProtcol(void);
void Can2_CommunicationProtcol(void);
/////////////////////////////////////////////////////////////////////////////
void KBrdIO_Init(void);
void LED_Toggle(void);
void GPIO_TogglePin(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin_x);
/////////////////////////////////////////////////////////////////////////////
uint8_t CUsart6_TxBuf[12];//0-1:IO×´Ì¬ 2£ºUSART×´Ì¬ 3£ºÎÂ¶È 4£ºÊª¶È 5-6£ºµçÑ¹ 7£ºCAN×´Ì¬
uint8_t CUsart6_RxBuf[10];
Timer RS485_RxTimeOut(0,500);
Timer RS485_ConnectTimeOut(0,10000);
////////////////////////////////////////////////////////////////////////////
uint8_t Can_Status=0x00;
CCanRxMailbox::MsgData CAN1rxBuf[10];
CCanRxMailbox IntBrdRxMailBox(CAN1rxBuf,10);
CCanRxMailbox::MsgData Can1RxBuf[10];
CCanRxMailbox Can1RxMailBox(Can1RxBuf,10);
CCanRxMailbox::MsgData Can2RxBuf[10];
CCanRxMailbox Can2RxMailBox(Can2RxBuf,10);
////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
uint16_t IO_Status=0x00;
uint8_t TempData=0x10;
uint8_t HumiData=0x00;
uint16_t VoltData=0x00;
///////////////////////////////////////////////////////////////////////////
uint8_t CUsart1_RXbuf[10];
CUsart CUsart1(USART1,CUsart1_RXbuf,10,115200);
void CUsart1_Test(void);

uint8_t CUsart2_RXbuf[10];
CUsart CUsart2(USART2,CUsart2_RXbuf,10,115200);
void CUsart2_Test(void);

uint8_t CUsart3_RXbuf[10];
CUsart CUsart3(USART3,CUsart3_RXbuf,10,115200);
void CUsart3_Test(void);

uint8_t CUsart4_RXbuf[10];
CUsart CUsart4(UART4,CUsart4_RXbuf,10,115200);
void CUsart4_Test(void);

uint8_t CUsart5_RXbuf[10];
CUsart CUsart5(UART5,CUsart5_RXbuf,10,115200);
void CUsart5_Test(void);

uint8_t CUsart6_RXbuf[10];
CUsart CUsart6(USART6,CUsart6_RXbuf,10,19200);

void IO_Test(void);
uint8_t Test_Buf[9];
///////////////////////////////////////////////////////////////////////////
int main(void)
{
	SystemCoreClockUpdate();
	BaseTimer::Instance()->initialize();
	
	CUsart1.InitSciGpio();
	CUsart1.InitSci();
	
	CUsart2.InitSciGpio();
	CUsart2.InitSci();
	
	CUsart3.InitSciGpio();
	CUsart3.InitSci();
	
	CUsart4.InitSciGpio();
	CUsart4.InitSci();
	
	CUsart5.InitSciGpio();
	CUsart5.InitSci();
	
	CUsart6.InitSciGpio();
	CUsart6.InitSci();
	
	CanRouter1.InitCanGpio(CCanRouter::GROUP_A11);
	CanRouter1.InitCan();
	CanRouter2.InitCanGpio(CCanRouter::GROUP_B12);
	CanRouter2.InitCan();
	IntBrdRxMailBox.setStdId(0x610);
	IntBrdRxMailBox.attachToRouter(CanRouter1);
	Can1RxMailBox.setStdId(0x612);
	Can1RxMailBox.attachToRouter(CanRouter1);
	Can2RxMailBox.setStdId(0x613);
	Can2RxMailBox.attachToRouter(CanRouter2);
	
	KBrdIO_Init();
	
	Timer HeartBeat(0,10000);
	
	GPIO_SetBits(GPIOE,GPIO_Pin_3);
	
	while(RS485_Connect()==1) ;
	GPIO_ResetBits(GPIOE,GPIO_Pin_3);
	memset(Test_Buf,0,9);
	while(1)
	{
		RS485_CommunicationProtcol(Test_Buf);
		CAN1_CommunicationProtcol();
		Can2_CommunicationProtcol();
		IO_Test();
		Test_Buf[0]=0x00;
		Test_Buf[1]=IO_Status;
		Test_Buf[2]=IO_Status>>8;
		
		CUsart1_Test();
		CUsart2_Test();
		CUsart3_Test();
		CUsart4_Test();
		CUsart5_Test();
		
		Test_Buf[4]=TempData;
		Test_Buf[5]=HumiData;
		Test_Buf[6]=(uint8_t)VoltData;
		Test_Buf[7]=VoltData>>8;
		Test_Buf[8]=Can_Status;
		
		
		if(HeartBeat.isAbsoluteTimeUp())
		{
			LED_Toggle();
			GPIO_TogglePin(GPIOE,GPIO_Pin_4);
			GPIO_ToggleBits(GPIOE,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
			//HeartBeat.reset();
		}
	}
}

void RS485_CommunicationProtcol(uint8_t* buf)
{
	uint8_t checksum=0;
	for(uint8_t m=0;m<9;++m)
	{
		CUsart6_TxBuf[m+2]=buf[m];
		checksum+=buf[m];
	}
	CUsart6_TxBuf[11]=checksum;
	CUsart6_TxBuf[0]=0xA5;
	CUsart6_TxBuf[1]=0x5A;
	CUsart6.send_Array(CUsart6_TxBuf,12);
	RS485_RxTimeOut.reset();
	while(CUsart6.get_BytesInRxFifo()==0) 
	{
		if(RS485_RxTimeOut.isAbsoluteTimeUp())
		{
			NVIC_SystemReset();
		}
	}
	CUsart6.read_RxFifo(CUsart6_RxBuf);
	CUsart6.clear_rxFifo();
	if(CUsart6_RxBuf[0]!=0xA5)
	{
		NVIC_SystemReset();
	}
}

int RS485_Connect(void)
{
	RS485_ConnectTimeOut.reset();
	CUsart6_TxBuf[0]=0xA5;
	CUsart6_TxBuf[1]=0x5A;
	CUsart6_TxBuf[2]=0x01;
	for(uint8_t i=3;i<7;++i)
	{
		CUsart6_TxBuf[i]=(UID>>((i-3)*8));
	}
	CUsart6_TxBuf[11]=CUsart6_TxBuf[0]+CUsart6_TxBuf[1]+CUsart6_TxBuf[2]+CUsart6_TxBuf[3]+CUsart6_TxBuf[4]+
										CUsart6_TxBuf[5]+CUsart6_TxBuf[6]+CUsart6_TxBuf[7]+CUsart6_TxBuf[8]+CUsart6_TxBuf[9]+
										CUsart6_TxBuf[10];
	CUsart6.send_Array(CUsart6_TxBuf,10);
	while(CUsart6.get_BytesInRxFifo()==0) 
	{
		if(RS485_ConnectTimeOut.isAbsoluteTimeUp())
			return 1;
	}
	for(uint8_t i=0;i<12;++i)
	{
		CUsart6_TxBuf[i]=0x00;
	}
	CUsart6.read_RxFifo(CUsart6_RXbuf);
	CUsart6.clear_rxFifo();
	if(CUsart6_RXbuf[0]!=0xA5)
		return 1;
	return 0;
}

void CAN1_CommunicationProtcol(void)
{
	static Timer CanRxTimeOut1(0,500);
	static Timer CanRxTimeOut2(0,500);
	static Timer CanTxTimeOut(0,100);
	//static uint8_t MsgRecv=0x00;
	CanRxMsg IntBrdRxMessage;
	CanRxMsg Can2RxMessage;
	CanRouter1.runTransmitter();
	CanRouter1.runReceiver();
	if(CanTxTimeOut.isAbsoluteTimeUp())
	{
		CanTxMsg TxMessage;
		TxMessage.StdId=0x611;
		TxMessage.ExtId=0x00;
		TxMessage.IDE=CAN_Id_Standard;
		TxMessage.RTR=CAN_RTR_DATA;
		TxMessage.DLC=1;
		TxMessage.Data[0]=0x55;
		CanRouter1.putMsg(TxMessage);
		TxMessage.StdId=0x613;
		TxMessage.Data[0]=0x66;
		CanRouter1.putMsg(TxMessage);
	}
	if(IntBrdRxMailBox.getMsg(&IntBrdRxMessage))
	{
		Can_Status|=0x01;
		CanRxTimeOut1.reset();
		IO_Status&=0x3FF;
		IO_Status|=IntBrdRxMessage.Data[0];
		IO_Status|=(uint16_t)IntBrdRxMessage.Data[1]<<8;
		TempData=IntBrdRxMessage.Data[2];
		HumiData=IntBrdRxMessage.Data[3];
		VoltData=IntBrdRxMessage.Data[4]+(IntBrdRxMessage.Data[5]<<8);
	}
	if(Can1RxMailBox.getMsg(&Can2RxMessage))
	{
		CanRxTimeOut2.reset();
		if(Can2RxMessage.Data[0]==0x77)
		{
			Can_Status|=0x02;
		}
	}
	if(CanRxTimeOut1.isAbsoluteTimeUp())
	{
		Can_Status&=0xFE;
	}
	if(CanRxTimeOut2.isAbsoluteTimeUp())
	{
		Can_Status&=0xFD;
	}
}

void Can2_CommunicationProtcol(void)
{
	CanRouter2.runTransmitter();
	CanRouter2.runReceiver();
	CanRxMsg Can2RxMsg;
	if(Can2RxMailBox.getMsg(&Can2RxMsg))
	{
		if(Can2RxMsg.Data[0]==0x66)
		{
			CanTxMsg TxMsg;
			TxMsg.StdId=0x612;
			TxMsg.ExtId=0x00;
			TxMsg.IDE=CAN_Id_Standard;
			TxMsg.RTR=CAN_RTR_DATA;
			TxMsg.DLC=0x01;
			TxMsg.Data[0]=0x77;
			CanRouter2.putMsg(TxMsg);
		}
	}
}

void KBrdIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE,ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
}

void LED_Toggle(void)
{
	if(GPIOE->ODR&0x04)
		GPIOE->ODR&=(~0x04);
	else
		GPIOE->ODR|=0x04;
}

void CUsart1_Test(void)
{
	static Timer _Timeup(0,500);
	static uint8_t _i=0;
	if(_Timeup.isAbsoluteTimeUp())
	{
		if(CUsart1.get_BytesInRxFifo()>0)
		{
			uint8_t rxtemp=0x00;
			if(CUsart1.read_RxFifo(&rxtemp)>0)
			{
				if(rxtemp==_i)
				{
					Test_Buf[3]|=0x01;
				}else{
					Test_Buf[3]&=(~0x01);
				}
			}else{
				Test_Buf[3]&=(~0x01);
			}
		}
		++_i;
		CUsart1.send_Array(&_i,1);
	}
}

void CUsart2_Test(void)
{
	static Timer _Timeup(0,500);
	static uint8_t _i=0;
	if(_Timeup.isAbsoluteTimeUp())
	{
		if(CUsart2.get_BytesInRxFifo()>0)
		{
			uint8_t rxtemp=0x00;
			if(CUsart2.read_RxFifo(&rxtemp)>0)
			{
				if(rxtemp==_i)
				{
					Test_Buf[3]|=0x02;
				}else{
					Test_Buf[3]&=(~0x02);
				}
			}else{
				Test_Buf[3]&=(~0x02);
			}
		}
		++_i;
		CUsart2.send_Array(&_i,1);
	}
}

void CUsart3_Test(void)
{
	static Timer _Timeup(0,500);
	static uint8_t _i=0;
	if(_Timeup.isAbsoluteTimeUp())
	{
		if(CUsart3.get_BytesInRxFifo()>0)
		{
			uint8_t rxtemp=0x00;
			if(CUsart3.read_RxFifo(&rxtemp)>0)
			{
				if(rxtemp==_i)
				{
					Test_Buf[3]|=0x04;
				}else{
					Test_Buf[3]&=(~0x04);
				}
			}else{
				Test_Buf[3]&=(~0x04);
			}
		}
		++_i;
		CUsart3.send_Array(&_i,1);
	}
}

void CUsart4_Test(void)
{
	static Timer _Timeup(0,500);
	static uint8_t _i=0;
	if(_Timeup.isAbsoluteTimeUp())
	{
		if(CUsart4.get_BytesInRxFifo()>0)
		{
			uint8_t rxtemp=0x00;
			if(CUsart4.read_RxFifo(&rxtemp)>0)
			{
				if(rxtemp==_i)
				{
					Test_Buf[3]|=0x08;
				}else{
					Test_Buf[3]&=(~0x08);
				}
			}else{
				Test_Buf[3]&=(~0x08);
			}
		}
		++_i;
		CUsart4.send_Array(&_i,1);
	}
}

void CUsart5_Test(void)
{
	static Timer _Timeup(0,500);
	static uint8_t _i=0;
	if(_Timeup.isAbsoluteTimeUp())
	{
		if(CUsart5.get_BytesInRxFifo()>0)
		{
			uint8_t rxtemp=0x00;
			if(CUsart5.read_RxFifo(&rxtemp)>0)
			{
				if(rxtemp==_i)
				{
					Test_Buf[3]|=0x10;
				}else{
					Test_Buf[3]&=(~0x10);
				}
			}else{
				Test_Buf[3]&=(~0x10);
			}
		}
		++_i;
		CUsart5.send_Array(&_i,1);
		USART_SendData(UART5,_i);
	}
}

void IO_Test(void)
{
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10)==RESET)
	{
		IO_Status|=0x0400;
	}
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11)==RESET)
	{
		IO_Status|=0x0800;
	}
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)==RESET)
	{
		IO_Status|=0x1000;
	}
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_13)==RESET)
	{
		IO_Status|=0x2000;
	}
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14)==RESET)
	{
		IO_Status|=0x4000;
	}
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_15)==RESET)
	{
		IO_Status|=0x8000;
	}
}

void GPIO_TogglePin(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin_x)
{
	if(GPIOx->ODR&GPIO_Pin_x)
	{
		GPIOx->ODR&=(~GPIO_Pin_x);
	}else{
		GPIOx->ODR|=GPIO_Pin_x;
	}
}

