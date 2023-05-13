#include <stdint.h>
#include "bitwise_operation.h"
#include "DIO.h"
#include "tm4c123gh6pm.h"


void delay(int num){
  for (int i = 0; i < 3180 * num; i++);
}

//PIN INIT

void DIO_Init(int port, int pin, int dir){
  Set_Bit(SYSCTL_RCGCGPIO_R, port);
  switch (port){
          case PORTA:
            Set_Bit(GPIO_PORTA_CR_R, pin);
            if(dir==OUTPUT){
                Set_Bit(GPIO_PORTA_DIR_R, pin);
            }
            else{
                Clear_Bit(GPIO_PORTA_DIR_R, pin);
                switch(dir){
                    case INPUT_PULLDOWN:
                        Set_Bit(GPIO_PORTA_PDR_R, pin);
                        break;
                    case INPUT_PULLUP:
                        Set_Bit(GPIO_PORTA_PUR_R, pin);
                        break;
                    case INPUT:
                        break;
                    default:
                        break;
                }
            }
            Set_Bit(GPIO_PORTA_DEN_R, pin);
            break;
          case PORTB:
          Set_Bit(GPIO_PORTB_CR_R, pin);
            if(dir==OUTPUT){
                Set_Bit(GPIO_PORTB_DIR_R, pin);
            }
            else{
                Clear_Bit(GPIO_PORTB_DIR_R, pin);
                switch(dir){
                    case INPUT_PULLDOWN:
                        Set_Bit(GPIO_PORTB_PDR_R, pin);
                        break;
                    case INPUT_PULLUP:
                        Set_Bit(GPIO_PORTB_PUR_R, pin);
                        break;
                    case INPUT:
                        break;
                    default:
                        break;
                }
            }
            Set_Bit(GPIO_PORTB_DEN_R, pin);
            break;            
            
        case PORTC:
            GPIO_PORTC_LOCK_R = 0x4C4F434B;
            Set_Bit(GPIO_PORTC_CR_R, pin);
            if(dir==OUTPUT){
                Set_Bit(GPIO_PORTC_DIR_R, pin);
            }
            else{
                Clear_Bit(GPIO_PORTC_DIR_R, pin);
                switch(dir){
                    case INPUT_PULLDOWN:
                        Set_Bit(GPIO_PORTC_PDR_R, pin);
                        break;
                    case INPUT_PULLUP:
                        Set_Bit(GPIO_PORTC_PUR_R, pin);
                        break;
                    case INPUT:
                        break;
                    default:
                        break;
                }
            }
            Set_Bit(GPIO_PORTC_DEN_R, pin);
            break;
        case PORTD:
            GPIO_PORTD_LOCK_R = 0x4C4F434B;
            Set_Bit(GPIO_PORTD_CR_R, pin);
            if(dir==OUTPUT){
                Set_Bit(GPIO_PORTD_DIR_R, pin);
            }
            else{
                Clear_Bit(GPIO_PORTD_DIR_R, pin);
                switch(dir){
                    case INPUT_PULLDOWN:
                        Set_Bit(GPIO_PORTD_PDR_R, pin);
                        break;
                    case INPUT_PULLUP:
                        Set_Bit(GPIO_PORTD_PUR_R, pin);
                        break;
                    case INPUT:
                        break;
                    default:
                        break;
                }
            }
            Set_Bit(GPIO_PORTD_DEN_R, pin);
            break;            
        case PORTE:
         
            Set_Bit(GPIO_PORTE_CR_R, pin);
            if(dir==OUTPUT){
                Set_Bit(GPIO_PORTE_DIR_R, pin);
            }
            else{
                Clear_Bit(GPIO_PORTE_DIR_R, pin);
                switch(dir){
                    case INPUT_PULLDOWN:
                        Set_Bit(GPIO_PORTE_PDR_R, pin);
                        break;
                    case INPUT_PULLUP:
                        Set_Bit(GPIO_PORTE_PUR_R, pin);
                        break;
                    case INPUT:
                        break;
                    default:
                        break;
                }
            }
            Set_Bit(GPIO_PORTE_DEN_R, pin);
            break;            
        case PORTF:
            GPIO_PORTF_LOCK_R = 0x4C4F434B;
            Set_Bit(GPIO_PORTF_CR_R, pin);
            if(dir==OUTPUT){
                Set_Bit(GPIO_PORTF_DIR_R, pin);
            }
            else{
                Clear_Bit(GPIO_PORTF_DIR_R, pin);
                switch(dir){
                    case INPUT_PULLDOWN:
                        Set_Bit(GPIO_PORTF_PDR_R, pin);
                        break;
                    case INPUT_PULLUP:
                        Set_Bit(GPIO_PORTF_PUR_R, pin);
                        break;
                    case INPUT:
                        break;
                    default:
                        break;
                }
            }
            Set_Bit(GPIO_PORTF_DEN_R, pin);
            break;
    default:
        break;

    }
}

//WRITE PIN
void DIO_WritePin(int port, int pin, int value){
    switch(port){
        case PORTA:
          if(value){
                Set_Bit(GPIO_PORTA_DATA_R, pin);
          }
          else{
                Clear_Bit(GPIO_PORTA_DATA_R, pin);
          }
            break;
          case PORTB:
          if(value){
                Set_Bit(GPIO_PORTB_DATA_R, pin);
          }
          else{
                Clear_Bit(GPIO_PORTB_DATA_R, pin);
          }
            break;          
        case PORTC:
          if(value){
                Set_Bit(GPIO_PORTC_DATA_R, pin);
          }
          else{
                Clear_Bit(GPIO_PORTC_DATA_R, pin);
          }
            break;     
        case PORTD:
          if(value){
                Set_Bit(GPIO_PORTD_DATA_R, pin);
          }
          else{
                Clear_Bit(GPIO_PORTD_DATA_R, pin);
          }
            break;          
         case PORTE:
          if(value){
                Set_Bit(GPIO_PORTE_DATA_R, pin);
          }
          else{
                Clear_Bit(GPIO_PORTE_DATA_R, pin);
          }
            break;
        case PORTF:
          if(value){
                Set_Bit(GPIO_PORTF_DATA_R, pin);
          }
          else{
                Clear_Bit(GPIO_PORTF_DATA_R, pin);
          }
            break;
    default:
        break;

    }
}

//WRITE PORT
void DIO_WritePort(int port, int value){
    switch (port){
    case PORTA:
        GPIO_PORTA_DATA_R = value;
        break;
    case PORTB:
        GPIO_PORTB_DATA_R = value;
        break;
    case PORTC:
        GPIO_PORTC_DATA_R = value;
        break;
    case PORTD:
        GPIO_PORTD_DATA_R = value;
        break;
    case PORTE:
        GPIO_PORTE_DATA_R = value;
        break;
    case PORTF:
        GPIO_PORTF_DATA_R = value;
        break;
    
    default:
        break;
    }
}

//READ PIN
int DIO_ReadPin(int port, int pin)
{
   switch(port)
  {
  case PORTA:
   return Get_Bit(GPIO_PORTA_DATA_R,pin); 
   break;
   
  case PORTB:
   return Get_Bit(GPIO_PORTB_DATA_R,pin);
   break;
   
  case PORTC:
   return Get_Bit(GPIO_PORTC_DATA_R,pin);
   break;
   
  case PORTD:
   return Get_Bit(GPIO_PORTD_DATA_R,pin);
   break;
   
  case PORTE:
   return Get_Bit(GPIO_PORTE_DATA_R,pin);
   break;
   
  case PORTF:
   return  Get_Bit(GPIO_PORTF_DATA_R,pin);
   break;
   
  default:
    return -1;
  }
}

//READ PORT
int DIO_ReadPort(int port)
{
   switch(port)
  {
  case PORTA:
    return GPIO_PORTA_DATA_R;
    break;
   
  case PORTB:
    return GPIO_PORTB_DATA_R;
    break;
    
  case PORTC:
    return GPIO_PORTC_DATA_R;
    break;
    
   case PORTD:
    return GPIO_PORTD_DATA_R;
    break;
    
   case PORTE:
    return GPIO_PORTE_DATA_R;
    break;
    
   case PORTF:
    return GPIO_PORTF_DATA_R;
    break;
    
   default:
    return -1;
  }
}