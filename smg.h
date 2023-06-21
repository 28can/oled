#ifndef __SMG_H__
#define __SMG_H__

#include "main.h"
#include "math.h"

//74HC138操作线
#define LED_A0(GPIO_PinState)       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PinState)  //A0地址线
#define LED_A1(GPIO_PinState)       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PinState)  //A1地址线
#define LED_A2(GPIO_PinState)       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PinState)  //A2地址线

//74HC595操作线
#define LED_DS(GPIO_PinState)       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PinState)   //数据线
#define LED_LCLK(GPIO_PinState)     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PinState)   //锁存时钟线
#define LED_SCK(GPIO_PinState)      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PinState)   //时钟线

void SMG_Init(void);
void LED_Write_Data(uint8_t duan, uint8_t wei);
void LED_Refresh(void);

void SMG_output(int bit, int number, int point);
void SMG_input_int(int number);
void SMG_input_float(double number);

#endif
