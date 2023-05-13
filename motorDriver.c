#include "motorDriver.h"

void motorInit(void) 
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
  {
  }
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
}

void motorMoveUp(void)
{
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PIN_2);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);
}

void motorMoveDown(void)
{
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);
}

void motorStop(void)
{
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);
}