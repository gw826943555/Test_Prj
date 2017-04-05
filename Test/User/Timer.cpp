#include "Timer.h"

CBaseTimer::CBaseTimer()// 要写完全!!!
{	
	_baseTimer = 0;	
	SystemCoreClockUpdate();
}

void CBaseTimer::initialize()
{	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); //21MHz. Different from F1.
	SysTick_Config(SystemCoreClock/8/1000);
	start();
}

void CBaseTimer::doUpdate()
{
	++_baseTimer;
	_baseTimer &= 0x7FFFFFFF;
}

Timer::Timer(int32_t delay,int32_t period)
{
	_timer = BaseTimer::Instance()->getTime();
	_counter = delay;
	_period = period;
}

void Timer::reset()
{
	_timer = BaseTimer::Instance()->getTime();
	_counter = _period;
}

bool Timer::isTimeUp()
{
	int32_t tempBaseTimer = BaseTimer::Instance()->getTime();
	if (_timer != tempBaseTimer)//be triggered
	{
		_timer = tempBaseTimer;
		if (--_counter <= 0)
		{
			_counter = _period;
			return true;
		}
	}
	return false;	
}

bool Timer::isAbsoluteTimeUp()
{
	int32_t tempBaseTimer = BaseTimer::Instance()->getTime();
	int32_t diff = tempBaseTimer - _timer;
	if ( diff < 0 )
	{
		diff &= 0x7FFFFFFF;
	}
	if (diff >= _period)
	{
		_timer = tempBaseTimer;
		return true;
	}
	return false;
}

void CPUTIMER0_ISR()
{
	BaseTimer::Instance()->doUpdate();
	SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
}
