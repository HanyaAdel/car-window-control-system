#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
//#define IN1 GPIO_PIN_0
//#define IN2 GPIO_PIN_1

void motorInit(void);
void motorMoveUp(void);
void motorMoveDown(void);
void motorStop(void);