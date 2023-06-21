#include "smg.h"

/*
    数码管驱动IO初始化函数
*/
void SMG_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();   //开启GPIOB时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();   //开启GPIOC时钟
    __HAL_RCC_AFIO_CLK_ENABLE();    //开启AFIO时钟
    __HAL_AFIO_REMAP_SWJ_NOJTAG();  //关闭jtag，使能SWD，可以用SWD模式调试 PB4做普通IO口使用,否则不能当IO使用
    
    //74HC595  DS-PB3 LCLK-PB4 SCK-PB5
    GPIO_Initure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
    
    //74HC138  A0-PC10 A1-PC11 A2-PC12
    GPIO_Initure.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);

}

/*
    微秒延时函数
*/
void delay_us(uint32_t nus)
{
	uint32_t Delay = nus * 168 / 4;
	

	do
	{
		__NOP();
	}
	
	while (Delay --);

}

/*
    74HC138驱动
    数码管位选
    num:要显示的数码管编号 0-7(共8个数码管)
*/
void LED_Wei(uint8_t num)
{
    LED_A0((GPIO_PinState)(num & 0x01));
    LED_A1((GPIO_PinState)((num & 0x02) >> 1));
    LED_A2((GPIO_PinState)((num & 0x04) >> 2));
}

/*
    74HC595驱动
    数码管显示
    duan:显示的段码
    wei:要显示的数码管编号 0-7(共8个数码管)
*/
void LED_Write_Data(uint8_t duan, uint8_t wei)
{
    uint8_t i;

    for ( i = 0; i < 8; i++) //先送段
    {
        LED_DS((GPIO_PinState)((duan >> i) & 0x01));
        LED_SCK((GPIO_PinState)0);
        delay_us(5);
        LED_SCK((GPIO_PinState)1);
    }
    
    LED_Wei(wei);//后选中位

}

/*
    74HC595驱动
    数码管刷新显示
*/
void LED_Refresh(void)
{
    LED_LCLK((GPIO_PinState)1);
    delay_us(5);
    LED_LCLK((GPIO_PinState)0);
}


/*
    共阴数字数组
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F, ., 全灭
*/
uint8_t smg_num[] = {0xfc, 0x60, 0xda, 0xf2, 0x66, 0xb6, 0xbe, 0xe0, 0xfe, 0xf6, 0xee, 0x3e, 0x9c, 0x7a, 0x9e, 0x8e, 0x01, 0x00};

/*
	数码管输出函数
    bit：显示在哪一位上
         1~8位
    number：显示什么数字、字母或字符
            对应共阴数字数组，0~9分别对应数字0~9，10对应字母A，以此类推，16对应字符.，17对应全灭
    point：是否显示小数点
           1为显示，0为不显示
*/
void SMG_output(int bit, int number, int point)
{
	if (point == 0)
	{
		LED_Write_Data(smg_num[number], bit - 1);
	}
	else if (point == 1)
	{
		LED_Write_Data(smg_num[number] | smg_num[16], bit - 1);
	}
	

	LED_Refresh();

}

/*
    一个用于简化代码量的数码管输出函数
*/
void SMG_output_loop(long long temp, int bit)
{
	SMG_output(1, 0, 1);

	for (int i = 2; i <= 7 - bit; i++)
	{
	    SMG_output(i, 0, 0);
	}
	
	for (int i = 8 - bit; i <= 8; i++)
	{
		SMG_output(i, temp / (long long)pow(10, 8 - i) % 10, 0);
	}

}

/*
    另一个用于简化代码量的数码管输出函数
*/
void SMG_output_loop_2(long long temp, int bit)
{
    int point = 0;

	for (int i = 1; i <= 8; i++)
	{
		if (bit - i + 1 == 7)
		{
			point = 1;
		}
		
		SMG_output(i, temp / (long long)pow(10, bit - i + 1) % 10, point);
	
		point = 0;
	}

}

/*
	整数型数据数码管输出函数
    number：要显示的整数
            1~99999999之间的所有整数
	注：此函数可能？有一个自带的延时效果
*/
void SMG_input_int(int number)
{
	for (int i = 1; i <= 8; i++)
	{
		SMG_output(i, number / (long long)pow(10, 8 - i) % 10, 0);
	}
}

/*
    浮点数型数据数码管输出函数
    number：要显示的浮点数
            0.0000001~99999999之间的所有浮点数
    注：当输入整数型数据时，也会被当成浮点数型数据进行处理，显示小数点
		此函数可能？有一个自带的延时效果
*/
void SMG_input_float(double number)
{
	int bit = 0, show = 1;
	long long temp;

    //判断是否小于最小输出值，小于则八位均显示-
    if (number < 0.0000001)
    {
    	show = 0;
    	
    	for (int i = 1; i < 9; i++)
    	{
    		LED_Write_Data(0x02, i);
    	}
    
    	LED_Refresh();
    }
    
    if (show == 1)
    {
    	temp = number * 10000000;
    
    	while (temp / 10 != 0)
    	{
    		bit ++;
    		temp = temp / 10;
    	}
    
    	temp = number * 10000000;
    
    	switch (bit)
    	{
    	case 0://0.0000001
    		SMG_output_loop(temp, bit);
    		break;
    
    	case 1://0.0000010
    		SMG_output_loop(temp, bit);
    		break;
    
    	case 2://0.0000100
    		SMG_output_loop(temp, bit);
    		break;
    
    	case 3://0.0001000
    		SMG_output_loop(temp, bit);
    		break;
    
    	case 4://0.0010000
    		SMG_output_loop(temp, bit);
    		break;
    
    	case 5://0.0100000
    		SMG_output_loop(temp, bit);
    		break;
    
    	case 6://0.1000000
    		SMG_output_loop(temp, bit);
    		break;
    
    	case 7://1.0000000
    		SMG_output_loop_2(temp, bit);
    		break;
    
    	case 8://10.000000
    		SMG_output_loop_2(temp, bit);
    		break;
    
    	case 9://100.00000
    		SMG_output_loop_2(temp, bit);
    		break;
    
    	case 10://1000.0000
    		SMG_output_loop_2(temp, bit);
    		break;
    
    	case 11://10000.000
    		SMG_output_loop_2(temp, bit);
    		break;
    
    	case 12://100000.00
    		SMG_output_loop_2(temp, bit);
    		break;
    
    	case 13://1000000.0
    		SMG_output_loop_2(temp, bit);
    		break;
    
    	case 14://10000000.
            SMG_output_loop_2(temp, bit);
    		break;
    
    	default:
    		break;
    	}
    }

}
