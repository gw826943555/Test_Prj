#ifndef _FW_CCAN_H
#define _FW_CCAN_H
#include "stdint.h"
#include "ringque.h"
#include "stm32f4xx.h"

class CCanRouter;
class CCanRxMailbox
{
public:
	typedef struct
	{
			uint8_t DLC;
			uint8_t Data[8];
	}MsgData;
	CCanRxMailbox(MsgData*, uint16_t rxQueSize);
	~CCanRxMailbox(){}
	void setStdId(uint16_t);
	void setExtId(uint32_t);
	uint16_t msgsInQue() const;
	bool getMsg(CanRxMsg*);
	bool getMsgData(MsgData*);
	virtual void pushMsg(const CanRxMsg&);
	void clear() {rxQue_.clear();}	
		
	bool attachToRouter(CCanRouter&);
	bool operator == (const CanRxMsg&);
	bool isIdEqual(const CanRxMsg&);
	bool isIdEqual(CCanRxMailbox*);
	bool IDE() {return IDE_;}
	uint32_t stdId() {return stdId_;}
	uint32_t extId() {return extId_;}
	uint8_t getRxOverflowcount(){return rxOverflowCount_;}

private:
	uint32_t stdId_;
	uint32_t extId_;
	uint8_t IDE_;
	ringque<MsgData> rxQue_;
	uint8_t rxOverflowCount_;
	bool isAttached;
};


class CCanRouter
{
	friend class CCanRxMailbox;
public:
	
	CCanRouter(CAN_TypeDef* CANx, 
		CanTxMsg* txQueBuf, 
		uint16_t txQueSize,
		uint32_t BaudRate = 250000);
	~CCanRouter(){}
	void InitCanGpio(int IOGroup);
	void InitCan();
	void setBaudrate(uint32_t);
	bool isInitialized(){return (isGpioInitialized_&&isCanInitialized_);}
	
	uint8_t getTxOverflowcount(){return txOverflowCount_;}

	enum IOGroup_Type
	{
		GROUP_B12 = 0,
		GROUP_A11,
		GROUP_D0,
		GROUP_B5,
		GROUP_B8
	};
	
	enum{MAX_MAILBOX_NUM = 32};
	void runTransmitter();
	void runReceiver();
	void putMsg(CanTxMsg&);
	uint16_t getTxQueFreeSize() {return txQue_.emptyElemsInQue();}
	uint16_t getMsgsInTxQue() {return txQue_.elemsInQue();}

private:
	bool attachMailbox(CCanRxMailbox* pMailbox);
	CAN_TypeDef * CANx_;
	uint32_t baudRate_;
	uint8_t CAN_Filter_FIFO_;
	uint8_t txOverflowCount_;
	bool isGpioInitialized_;
	bool isCanInitialized_;

private:
	ringque<CanTxMsg> txQue_;
	CCanRxMailbox* mailboxTab[MAX_MAILBOX_NUM];
	uint16_t mailboxNum_;
};

extern CCanRouter CanRouter1;
extern CCanRouter CanRouter2;
#endif
//end of file
