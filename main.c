

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "motorDriver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "tm4c123gh6pm.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "DIO.h"


SemaphoreHandle_t xUnlockSemaphore;
SemaphoreHandle_t xLockSemaphore;

SemaphoreHandle_t xOpenWindowSemaphore;
//SemaphoreHandle_t xStopWindowSemaphore;
SemaphoreHandle_t xCloseWindowSemaphore;



SemaphoreHandle_t xAutoOpenWindowSemaphore;
SemaphoreHandle_t xAutoCloseWindowSemaphore;


SemaphoreHandle_t xOpenWindowDriverSemaphore;
//SemaphoreHandle_t xStopWindowDriverSemaphore;
SemaphoreHandle_t xCloseWindowDriverSemaphore;


SemaphoreHandle_t xAutoOpenWindowDriverSemaphore;
SemaphoreHandle_t xAutoCloseWindowDriverSemaphore;

SemaphoreHandle_t xJamProtectionSemaphore;

SemaphoreHandle_t xLimitSwitchUpSemaphore;
SemaphoreHandle_t xLimitSwitchDownSemaphore;

//SemaphoreHandle_t xReleaseLimitSwitchUpSemaphore;
//SemaphoreHandle_t xReleaseLimitSwitchDownSemaphore;

QueueHandle_t xStateQueuePassenger;
QueueHandle_t xStateQueueDriver;
QueueHandle_t xlockingQueue;
QueueHandle_t xLimitSwitchQueue;

QueueHandle_t xStateQueueAutoOPenPassenger;
QueueHandle_t xStateQueueAutoClosePassenger;

QueueHandle_t xStateQueueAutoOpenDriver;
QueueHandle_t xStateQueueAutoCloseDriver;

xSemaphoreHandle motorMutex;

TickType_t startTimeDriver, startTimePassenger;

static void Lock(void *pvParameters);

static void OpenWindow(void *pvParameters);
static void CloseWindow(void *pvParameters);

static void AutoOpenWindow(void *pvParameters);
static void AutoCloseWindow(void *pvParameters);

static void OpenWindowDriver(void *pvParameters);
static void CloseWindowDriver(void *pvParameters);

static void AutoOpenWindowDriver(void *pvParameters);
static void AutoCloseWindowDriver(void *pvParameters);

static void JamProtection(void *pvParameters);

static void LimitSwitchUp(void *pvParameters);
static void LimitSwitchDown(void *pvParameters);

//static void ReleaseLimitSwitchUp(void *pvParameters);
//static void ReleaseLimitSwitchDown(void *pvParameters);

static void Unlock(void *pvParameters);



//static void StopWindow( void *pvParameters );
//static void StopWindowDriver( void *pvParameters );
int counter = 0;

void GPIOF_Handler(void);
void GPIOA_Handler(void);
void GPIOE_Handler(void);
void GPIOC_Handler(void);
void Timer0AIntHandler();


void initTimer0(){
  //connect clock to TIEMR0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  //wait for initialization
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
  // enable interrupt flag for TIEMR0A after elapse
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  //configer TIEMR0A as a periodic timer
  TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
  //set reset value (equivalent to 1ms)
  TimerLoadSet(TIMER0_BASE, TIMER_A, 16000000 / 1000);
  //enable interrupts for TIEMR0A
	
  IntEnable(INT_TIMER0A_TM4C123);
  // define the interrupt handler for TIEMR0A (the handler's address is sent to the function)
	TimerIntRegister(TIMER0_BASE,TIMER_A,Timer0AIntHandler);
}

int main(void)
{

	motorMutex = xSemaphoreCreateMutex();

	xUnlockSemaphore = xSemaphoreCreateBinary();
	xLockSemaphore = xSemaphoreCreateBinary();
	xOpenWindowSemaphore = xSemaphoreCreateBinary();
	xCloseWindowSemaphore = xSemaphoreCreateBinary();
	
	xAutoOpenWindowSemaphore = xSemaphoreCreateBinary();
	xAutoCloseWindowSemaphore = xSemaphoreCreateBinary();
	//xStopWindowSemaphore = xSemaphoreCreateBinary();
	//xStopWindowDriverSemaphore = xSemaphoreCreateBinary();

	xOpenWindowDriverSemaphore = xSemaphoreCreateBinary();
	xCloseWindowDriverSemaphore = xSemaphoreCreateBinary();
	
	xAutoOpenWindowDriverSemaphore = xSemaphoreCreateBinary();
	xAutoCloseWindowDriverSemaphore = xSemaphoreCreateBinary();

	xJamProtectionSemaphore = xSemaphoreCreateBinary();

	xLimitSwitchUpSemaphore = xSemaphoreCreateBinary();
	xLimitSwitchDownSemaphore = xSemaphoreCreateBinary();
	
	//xReleaseLimitSwitchUpSemaphore = xSemaphoreCreateBinary();
	//xReleaseLimitSwitchDownSemaphore = xSemaphoreCreateBinary();

	xStateQueuePassenger = xQueueCreate(1, sizeof(int));
	xStateQueueDriver = xQueueCreate(1, sizeof(int));

	xStateQueueAutoOPenPassenger = xQueueCreate(1, sizeof(int));
	xStateQueueAutoClosePassenger = xQueueCreate(1, sizeof(int));
	xStateQueueAutoOpenDriver = xQueueCreate(1, sizeof(int));
	xStateQueueAutoCloseDriver = xQueueCreate(1, sizeof(int));


	xlockingQueue = xQueueCreate(1, sizeof(int));

	xLimitSwitchQueue = xQueueCreate(1, sizeof(int));

	if (xLimitSwitchQueue != NULL)
	{
		int value = 1;

		xQueueSendToBack(xlockingQueue, &value, portMAX_DELAY);
		// initializing ports
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		
		while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
			;
		initTimer0();
		motorInit();
		// port F, pin 0: lock
		// port F, pin 3: jam button
		// port A, pin 4: open passenger
		// port A, pin 5: close passenger
		// port E, pin 2: limit switch up
		// port E, pin 3: limit switch down

		// c 4 driver open
		// c 5 driver close
		
		
		// a 7 - auto driver open
		// C 7 - auto driver close
		
		// f 1 - auto passenger open
		// f4 - auto passenger close

		GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7 | GPIO_PIN_5 | GPIO_PIN_4);
		GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_7 | GPIO_PIN_4 | GPIO_PIN_5);
		GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3);
		GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4);

		GPIOUnlockPin(GPIO_PORTA_BASE, GPIO_PIN_7 | GPIO_PIN_5 | GPIO_PIN_4);
		GPIOUnlockPin(GPIO_PORTC_BASE, GPIO_PIN_7 | GPIO_PIN_4 | GPIO_PIN_5);
		GPIOUnlockPin(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3);
		GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4);

		// bottons are pull up
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

		GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

		GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

		GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

		// Interrupt
		IntMasterEnable();
		GPIOIntEnable(GPIO_PORTA_BASE, GPIO_INT_PIN_7 | GPIO_INT_PIN_5 | GPIO_INT_PIN_4);
		GPIOIntEnable(GPIO_PORTC_BASE,GPIO_INT_PIN_7 | GPIO_INT_PIN_5 | GPIO_INT_PIN_4);
		GPIOIntEnable(GPIO_PORTE_BASE, GPIO_INT_PIN_2 | GPIO_INT_PIN_3);
		GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_1 | GPIO_INT_PIN_3 | GPIO_INT_PIN_4);

		GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_BOTH_EDGES);
		GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_RISING_EDGE);
		GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);
		GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);

		GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_RISING_EDGE);
		GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_BOTH_EDGES);
		GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES);

		GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES);
		GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_BOTH_EDGES);
		GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_RISING_EDGE);

		GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_BOTH_EDGES);
		GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_BOTH_EDGES);

		GPIOIntRegister(GPIO_PORTF_BASE, GPIOF_Handler);
		GPIOIntRegister(GPIO_PORTA_BASE, GPIOA_Handler);
		GPIOIntRegister(GPIO_PORTE_BASE, GPIOE_Handler);
		GPIOIntRegister(GPIO_PORTC_BASE, GPIOC_Handler);
		IntPrioritySet(INT_GPIOF, 0xE0);
		IntPrioritySet(INT_GPIOA, 0xE0);
		IntPrioritySet(INT_GPIOC, 0xE0);
		IntPrioritySet(INT_GPIOE, 0xE0);

		xTaskCreate(LimitSwitchUp, "Limit switch up", 240, "limit switch up", 7, NULL);
		xTaskCreate(LimitSwitchDown, "Limit switch down", 240, "limit switch down", 7, NULL);
		
		//xTaskCreate(ReleaseLimitSwitchDown, "Release Limit switch down", 240, "release limit switch down", 5, NULL);
		//xTaskCreate(ReleaseLimitSwitchUp, "Limit switch down", 240, "release limit switch down", 5, NULL);

		xTaskCreate(JamProtection, "Jam Protection", 240, "Jam Protection", 6, NULL);
		
		
		xTaskCreate(Lock, "Lock", 240, "Task 1", 5, NULL);
		xTaskCreate( Unlock, "Unlock", 240, "Task 2", 5, NULL );
		xTaskCreate(OpenWindow, "Open Window", 240, "Open Window", 2, NULL);
		xTaskCreate(CloseWindow, "Close Window", 240, "Close Window", 2, NULL);
		
		xTaskCreate(AutoOpenWindow, "Auto Open Window", 240, "Auto Open Window", 2, NULL);
		xTaskCreate(AutoCloseWindow, "Auto Close Window", 240, "Auto Close Window", 2, NULL);
		//xTaskCreate( StopWindow, "Stop Window", 240, "Stop Window", 2, NULL);

		xTaskCreate(CloseWindowDriver, "Close Window Driver", 240, "Close Window Driver", 3, NULL);
		xTaskCreate(OpenWindowDriver, "Open Window Driver", 240, "Open Window Driver", 3, NULL);
		
		xTaskCreate(AutoCloseWindowDriver, "Auto Close Window Driver", 240, "Auto Close Window Driver", 3, NULL);
		xTaskCreate(AutoOpenWindowDriver, "Auto Open Window Driver", 240, "Auto Open Window Driver", 3, NULL);
		//xTaskCreate( StopWindowDriver, "Stop Window Driver", 240, "Stop Window Driver", 2, NULL);

		vTaskStartScheduler();
	}

	for (;;)
		;
}


//Timer0 interrupt handler
void Timer0AIntHandler(void)
{
  //Clear the timer0 interrupt flag.
	counter ++;
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

		// a 7 - auto driver open
		// C 7 - auto driver close
		// f 1 - auto passenger open
		// f4 - auto passenger close


// driver open, close, auto driver close
void GPIOC_Handler(void)
{
	delay(10);
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	int state = GPIOIntStatus(GPIO_PORTC_BASE, true);
	int value;
	int currState;
	int check_edge = 0;
	switch (state)
	{
	case 16:
		// open driver
		check_edge = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_4);
		if (check_edge == GPIO_PIN_4)
		{ // positive edge detected

			if (uxQueueMessagesWaitingFromISR(xStateQueueDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			}
			value = 300;
			xQueueSendToBackFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			startTimeDriver = xTaskGetTickCountFromISR();
			xSemaphoreGiveFromISR(xOpenWindowDriverSemaphore, &xHigherPriorityTaskWoken);
		
		// stop auto close when opening the window 
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoCloseDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoCloseDriver, &value, &xHigherPriorityTaskWoken);
			}
		
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoClosePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoClosePassenger, &value, &xHigherPriorityTaskWoken);
			}
		}
		else
		{
			//xSemaphoreGiveFromISR(xStopWindowDriverSemaphore, &xHigherPriorityTaskWoken);

			if (uxQueueMessagesWaitingFromISR(xStateQueueDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			}
		}

		break;
	case 32:
		// close driver
		check_edge = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_5);
		if (check_edge == GPIO_PIN_5)
		{ // positive edge detected
			if (uxQueueMessagesWaitingFromISR(xStateQueueDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			}
			value = 400;
			xQueueSendToBackFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			xSemaphoreGiveFromISR(xCloseWindowDriverSemaphore, &xHigherPriorityTaskWoken);
		
		
		// stop auto open when closing the window 
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOpenDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOpenDriver, &value, &xHigherPriorityTaskWoken);
			}
		
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOPenPassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOPenPassenger, &value, &xHigherPriorityTaskWoken);
			}
		}
		else
		{
			//xSemaphoreGiveFromISR(xStopWindowDriverSemaphore, &xHigherPriorityTaskWoken);

			if (uxQueueMessagesWaitingFromISR(xStateQueueDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			}
		}

		break;
	case 128:
		// auto driver close
	
	if (uxQueueMessagesWaitingFromISR(xStateQueueAutoCloseDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoCloseDriver, &value, &xHigherPriorityTaskWoken);
			}
								if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOpenDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOpenDriver, &value, &xHigherPriorityTaskWoken);
			}
	value = 400;
	xQueueSendToBackFromISR(xStateQueueAutoCloseDriver, &value, &xHigherPriorityTaskWoken);		
	xSemaphoreGiveFromISR(xAutoCloseWindowDriverSemaphore, &xHigherPriorityTaskWoken);
			
		break;
		
	}

	GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_7 | GPIO_INT_PIN_5 | GPIO_INT_PIN_4);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

// limit switch up
void GPIOE_Handler(void)
{
	delay(10);
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	int state = GPIOIntStatus(GPIO_PORTE_BASE, true);
	int value = 1;

	int currState;
	switch (state)
	{
		int check_edge;
	case 4:
		// limit switch up
		check_edge = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2);
		if (check_edge == GPIO_PIN_2)
		{
			if (uxQueueMessagesWaitingFromISR(xLimitSwitchQueue) == 0)
			{
				xQueueSendToBackFromISR(xLimitSwitchQueue, &value, &xHigherPriorityTaskWoken);
			}
			//xSemaphoreGiveFromISR(xLimitSwitchUpSemaphore, &xHigherPriorityTaskWoken);
		}
		else
		{
			if (uxQueueMessagesWaitingFromISR(xLimitSwitchQueue) == 1)
			{
				xQueueReceiveFromISR(xLimitSwitchQueue, &value, &xHigherPriorityTaskWoken);
			}
			
					int value;
					if (uxQueueMessagesWaitingFromISR( xStateQueueDriver) == 1){
						xQueueReceiveFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
						xQueueSendToBackFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
						if (value == 400){
							xSemaphoreGiveFromISR(xCloseWindowDriverSemaphore, &xHigherPriorityTaskWoken);
							
						}
					}
					if (uxQueueMessagesWaitingFromISR( xStateQueuePassenger) == 1){
						xQueueReceiveFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
						xQueueSendToBackFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
						if (value == 200){
							xSemaphoreGiveFromISR(xCloseWindowSemaphore, &xHigherPriorityTaskWoken);
							
						}
						
					}
		}

		break;

	case 8:
		// limit switch down
		check_edge = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3);
		value = 2;
		if (check_edge == GPIO_PIN_3)
		{
			if (uxQueueMessagesWaitingFromISR(xLimitSwitchQueue) == 0)
			{
				xQueueSendToBackFromISR(xLimitSwitchQueue, &value, &xHigherPriorityTaskWoken);
			}
			//xSemaphoreGiveFromISR(xLimitSwitchDownSemaphore, &xHigherPriorityTaskWoken);
		}
		else
		{
			if (uxQueueMessagesWaitingFromISR(xLimitSwitchQueue) == 1)
			{
				xQueueReceiveFromISR(xLimitSwitchQueue, &value, &xHigherPriorityTaskWoken);
			}
			
			
		int value;
		if (uxQueueMessagesWaitingFromISR( xStateQueueDriver) == 1){
			xQueueReceiveFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			xQueueSendToBackFromISR(xStateQueueDriver, &value, &xHigherPriorityTaskWoken);
			if (value == 300){
				xSemaphoreGiveFromISR(xOpenWindowDriverSemaphore, &xHigherPriorityTaskWoken);
				
			}
		}
		if (uxQueueMessagesWaitingFromISR( xStateQueuePassenger) == 1){
			xQueueReceiveFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
			xQueueSendToBackFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
			if (value == 100){
				xSemaphoreGiveFromISR(xOpenWindowSemaphore, &xHigherPriorityTaskWoken);
				
			}
			
		}
		
		
		
		
		}

		break;
	}

	GPIOIntClear(GPIO_PORTE_BASE, GPIO_INT_PIN_2 | GPIO_INT_PIN_3);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
// passenger open/ close button / auto driver open
void GPIOA_Handler(void)
{
	delay(10);
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	int state = GPIOIntStatus(GPIO_PORTA_BASE, true);
	int value;
	int currState;
	int check_edge = 0;
	switch (state)
	{

	case 32:
		// passenger close
		check_edge = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5);
		if (check_edge == GPIO_PIN_5)
		{
			if (uxQueueMessagesWaitingFromISR(xStateQueuePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
			}

			currState = 200;

			xQueueSendToBackFromISR(xStateQueuePassenger, &currState, &xHigherPriorityTaskWoken);

			xSemaphoreGiveFromISR(xCloseWindowSemaphore, &xHigherPriorityTaskWoken);
		
		
		// stop auto open when closing the window 
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOpenDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOpenDriver, &value, &xHigherPriorityTaskWoken);
			}
		
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOPenPassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOPenPassenger, &value, &xHigherPriorityTaskWoken);
			}
		
		}
		else
		{
			if (uxQueueMessagesWaitingFromISR(xlockingQueue) == 1)
			{
				//xSemaphoreGiveFromISR(xStopWindowSemaphore, &xHigherPriorityTaskWoken);
			}

			int value;
			if (uxQueueMessagesWaitingFromISR(xStateQueuePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
			}
			
		}

		break;
	case 16:
		// passenger open
		check_edge = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_4);
		if (check_edge == GPIO_PIN_4)
		{
			if (uxQueueMessagesWaitingFromISR(xStateQueuePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
			}

			currState = 100;
			xQueueSendToBackFromISR(xStateQueuePassenger, &currState, &xHigherPriorityTaskWoken);
			startTimePassenger = xTaskGetTickCountFromISR();
			xSemaphoreGiveFromISR(xOpenWindowSemaphore, &xHigherPriorityTaskWoken);
		
		
		
		// stop auto close when opening the window 
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoCloseDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoCloseDriver, &value, &xHigherPriorityTaskWoken);
			}
		
			if (uxQueueMessagesWaitingFromISR(xStateQueueAutoClosePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoClosePassenger, &value, &xHigherPriorityTaskWoken);
			}
		}
		else
		{
			if (uxQueueMessagesWaitingFromISR(xlockingQueue) == 1)
			{
				//xSemaphoreGiveFromISR(xStopWindowSemaphore, &xHigherPriorityTaskWoken);
			}

			int value;
			if (uxQueueMessagesWaitingFromISR(xStateQueuePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueuePassenger, &value, &xHigherPriorityTaskWoken);
			}
		}

		break;
		
		// auto open driver
	case 128:
	
	if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOpenDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOpenDriver, &value, &xHigherPriorityTaskWoken);
			}
					if (uxQueueMessagesWaitingFromISR(xStateQueueAutoCloseDriver) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoCloseDriver, &value, &xHigherPriorityTaskWoken);
			}
		value = 300;
		xQueueSendToBackFromISR(xStateQueueAutoOpenDriver, &value, &xHigherPriorityTaskWoken);	
	
		xSemaphoreGiveFromISR(xAutoOpenWindowDriverSemaphore, &xHigherPriorityTaskWoken);
		break;
	}

	GPIOIntClear(GPIO_PORTA_BASE, GPIO_INT_PIN_7 | GPIO_INT_PIN_5 | GPIO_INT_PIN_4);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

// locking and unlocking / auto passenger close / auto passenger open
void GPIOF_Handler(void)
{
	delay(10);
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	int state = GPIOIntStatus(GPIO_PORTF_BASE, true);
	int check_edge = 0;
	switch (state)
	{
		int value;
	case 1:
		// locking
		check_edge = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
		if (check_edge == GPIO_PIN_0)
		{
			if (uxQueueMessagesWaitingFromISR(xlockingQueue) == 1)
			{
				xQueueReceiveFromISR(xlockingQueue, &value, &xHigherPriorityTaskWoken);
			}
			xSemaphoreGiveFromISR(xLockSemaphore, &xHigherPriorityTaskWoken);
		}
		else
		{
			if (uxQueueMessagesWaitingFromISR(xlockingQueue) == 0)
			{
				value = 1;
				xQueueSendToBackFromISR(xlockingQueue, &value, &xHigherPriorityTaskWoken);
			}
			xSemaphoreGiveFromISR(xUnlockSemaphore, &xHigherPriorityTaskWoken);
		}

		break;
	case 2:
		// auto passenger open
	
	if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOPenPassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOPenPassenger, &value, &xHigherPriorityTaskWoken);
			}
			
		if (uxQueueMessagesWaitingFromISR(xStateQueueAutoClosePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoClosePassenger, &value, &xHigherPriorityTaskWoken);
			}
		value = 100;
		xQueueSendToBackFromISR(xStateQueueAutoOPenPassenger, &value, &xHigherPriorityTaskWoken);	
	
	xSemaphoreGiveFromISR(xAutoOpenWindowSemaphore, &xHigherPriorityTaskWoken);
		break;
	case 8:
		// jam button
		xSemaphoreGiveFromISR(xJamProtectionSemaphore, &xHigherPriorityTaskWoken);
		break;
	case 16:
		// auto passenger close
		if (uxQueueMessagesWaitingFromISR(xStateQueueAutoClosePassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoClosePassenger, &value, &xHigherPriorityTaskWoken);
			}
		if (uxQueueMessagesWaitingFromISR(xStateQueueAutoOPenPassenger) == 1)
			{
				xQueueReceiveFromISR(xStateQueueAutoOPenPassenger, &value, &xHigherPriorityTaskWoken);
			}
				
		value = 200;
		xQueueSendToBackFromISR(xStateQueueAutoClosePassenger, &value, &xHigherPriorityTaskWoken);	
	
	
	xSemaphoreGiveFromISR(xAutoCloseWindowSemaphore, &xHigherPriorityTaskWoken);
		break;
	}

	GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4 | GPIO_INT_PIN_3 | GPIO_INT_PIN_1| GPIO_INT_PIN_0);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

static void LimitSwitchUp(void *pvParameters)
{
	xSemaphoreTake(xLimitSwitchUpSemaphore, 0);
	for (;;)
	{
		xSemaphoreTake(xLimitSwitchUpSemaphore, portMAX_DELAY);
		motorStop();
	}
}

static void LimitSwitchDown(void *pvParameters)
{
	xSemaphoreTake(xLimitSwitchDownSemaphore, 0);
	for (;;)
	{
		xSemaphoreTake(xLimitSwitchDownSemaphore, portMAX_DELAY);
		motorStop();
	}
}

static void JamProtection(void *pvParameters)
{
	xSemaphoreTake(xJamProtectionSemaphore, 0);
	for (;;)
	{

		xSemaphoreTake(xJamProtectionSemaphore, portMAX_DELAY);
		
		
		TimerEnable(TIMER0_BASE, TIMER_A);
		motorMoveDown();
		counter = 0;
		
		
			
			while(1){
				if (counter == 3000){
					counter = 0;
					break;
				}				
				int limSwitch = 0;
				xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
				if (limSwitch == 2)
				{
					break; 
				}
			}
			
		TimerDisable(TIMER0_BASE, TIMER_A);
		//delay(2000);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorStop();
		}
		// xSemaphoreGive( motorMutex );
	}
}
/*static void StopWindowDriver(void *pvParameters)
{
	xSemaphoreTake(xStopWindowDriverSemaphore, 0);
	for (;;)
	{

		xSemaphoreTake(xStopWindowDriverSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorStop();
		}
		// xSemaphoreGive( motorMutex );
	}
}*/

static void AutoOpenWindowDriver(void *pvParameters)
{
	xSemaphoreTake(xAutoOpenWindowDriverSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{

		xSemaphoreTake(xAutoOpenWindowDriverSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveDown();
		}
		// xSemaphoreGive( motorMutex );

		
		if (uxQueueMessagesWaiting(xStateQueueAutoOpenDriver) == 1)
		{
			int limSwitch = 0;
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if (limSwitch != 2)
			{
				xSemaphoreGive(xAutoOpenWindowDriverSemaphore);
			}
		}
		}
	
		

}

static void OpenWindowDriver(void *pvParameters)
{
	xSemaphoreTake(xOpenWindowDriverSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{

		xSemaphoreTake(xOpenWindowDriverSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveDown();
		}
		// xSemaphoreGive( motorMutex );

		int value, limSwitch = 0;
		if (xQueuePeek(xStateQueueDriver, &value, 0))
		{
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if (value == 300 && limSwitch != 2)
			{
				xSemaphoreGive(xOpenWindowDriverSemaphore);
			}
		}
	}
}

static void AutoCloseWindowDriver(void *pvParameters)
{
	xSemaphoreTake(xAutoCloseWindowDriverSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{
		xSemaphoreTake(xAutoCloseWindowDriverSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveUp();
		}
		// xSemaphoreGive( motorMutex );
		
		
		if (uxQueueMessagesWaiting(xStateQueueAutoCloseDriver) == 1)
		{
		int limSwitch = 0;
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if ( limSwitch != 1)
			{
				xSemaphoreGive(xAutoCloseWindowDriverSemaphore);
			}
		}
		
		
		

		}
}


static void CloseWindowDriver(void *pvParameters)
{
	xSemaphoreTake(xCloseWindowDriverSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{
		xSemaphoreTake(xCloseWindowDriverSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveUp();
		}
		// xSemaphoreGive( motorMutex );
		int value, limSwitch = 0;
		if (xQueuePeek(xStateQueueDriver, &value, 0))
		{
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if (value == 400 && limSwitch != 1)
			{
				xSemaphoreGive(xCloseWindowDriverSemaphore);
			}
		}
	}
}

/*static void StopWindow( void *pvParameters ){
	xSemaphoreTake(xStopWindowSemaphore, 0);
	for(;;){

		xSemaphoreTake(xStopWindowSemaphore, portMAX_DELAY);
		
		int value;
	
		if (uxQueueMessagesWaiting( xStateQueuePassenger) == 1){
			xQueueReceive(xStateQueuePassenger, &value, portMAX_DELAY);
			
		}
		//xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorStop();
		}
		//xSemaphoreGive( motorMutex );
		
		
	}
}*/

static void Lock(void *pvParameters)
{

	xSemaphoreTake(xLockSemaphore, 0);
	for (;;)
	{
		xSemaphoreTake(xLockSemaphore, portMAX_DELAY);

		if (uxQueueMessagesWaiting(xStateQueueDriver) == 0)
		{
			motorStop();
		}
	}
}

static void AutoOpenWindow(void *pvParameters)
{
	xSemaphoreTake(xAutoOpenWindowSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{

		xSemaphoreTake(xAutoOpenWindowSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveDown();
		}
		// xSemaphoreGive( motorMutex );
		
		if (uxQueueMessagesWaiting(xStateQueueAutoOPenPassenger) == 1 )
		{
		int limSwitch = 0;
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if (uxQueueMessagesWaiting(xlockingQueue) == 1 && limSwitch != 2)
			{
				xSemaphoreGive(xAutoOpenWindowSemaphore);
			}
		}		


		}
}

static void OpenWindow(void *pvParameters)
{
	xSemaphoreTake(xOpenWindowSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{

		xSemaphoreTake(xOpenWindowSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveDown();
		}
		// xSemaphoreGive( motorMutex );

		int value, limSwitch = 0;
		if (xQueuePeek(xStateQueuePassenger, &value, 0))
		{
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if (uxQueueMessagesWaiting(xlockingQueue) == 1 && limSwitch != 2 && value == 100)
			{
				xSemaphoreGive(xOpenWindowSemaphore);
			}
		}
	}
}

static void AutoCloseWindow(void *pvParameters)
{
	xSemaphoreTake(xAutoCloseWindowSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{
		xSemaphoreTake(xAutoCloseWindowSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveUp();
		}
		// xSemaphoreGive( motorMutex );

		int limSwitch = 0;
		
		if (uxQueueMessagesWaiting(xStateQueueAutoClosePassenger) == 1)
		{
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if (uxQueueMessagesWaiting(xlockingQueue) == 1 &&
				limSwitch != 1)
			{
				xSemaphoreGive(xAutoCloseWindowSemaphore);
			}
		}
		
	}
		
		

}

static void CloseWindow(void *pvParameters)
{
	xSemaphoreTake(xCloseWindowSemaphore, 0);

	const portTickType delay = 1000 / portTICK_RATE_MS;
	for (;;)
	{
		xSemaphoreTake(xCloseWindowSemaphore, portMAX_DELAY);

		// xSemaphoreTake( motorMutex, portMAX_DELAY );
		{
			motorMoveUp();
		}
		// xSemaphoreGive( motorMutex );

		int value, limSwitch = 0;
		if (xQueuePeek(xStateQueuePassenger, &value, 0))
		{
			xQueuePeek(xLimitSwitchQueue, &limSwitch, 0);
			if (uxQueueMessagesWaiting(xlockingQueue) == 1 &&
				limSwitch != 1 && value == 200)
			{
				xSemaphoreGive(xCloseWindowSemaphore);
			}
		}
	}
}

static void Unlock( void *pvParameters )
{
	xSemaphoreTake(xUnlockSemaphore, 0);
	for( ;; )
	{
		xSemaphoreTake(xUnlockSemaphore, portMAX_DELAY);
	
		
		//int x = 1;
		//xQueueOverwrite(xlockingQueue, &x);
		
		int value;
		if (uxQueueMessagesWaiting( xStateQueuePassenger) == 1){
			xQueueReceive(xStateQueuePassenger, &value, portMAX_DELAY);
			xQueueSendToBack(xStateQueuePassenger, &value, portMAX_DELAY);
			if (value == 100){
				xSemaphoreGive(xOpenWindowSemaphore);
				
			}
			if (value == 200){
				xSemaphoreGive(xCloseWindowSemaphore);
				
			}			
			
		}

	}
}

void vApplicationIdleHook(void)
{
	motorStop();
}
