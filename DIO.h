#define PORTA 0
#define PORTB 1
#define PORTC 2
#define PORTD 3
#define PORTE 4
#define PORTF 5

// #define IN 0
// #define OUT 1

#define BUTTON_2 0
#define RED 1
#define BLUE 2
#define GREEN 3
#define BUTTON_1 4

#define HIGH 1
#define LOW 0

#define INPUT 0             //FLOATING
#define INPUT_PULLUP 1
#define INPUT_PULLDOWN 2
#define OUTPUT 3

void DIO_Init(int port, int pin, int dir);
void DIO_WritePin(int port, int pin, int value);
void DIO_WritePort(int port, int value);
int DIO_ReadPin(int port, int pin);
int DIO_ReadPort(int port);
void delay(int num);
