/******************************************************************************
 * Copyright (C) 2021, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************/
 
/******************************************************************************
 ** @file main.c
 **
 ** @brief Source file for MAIN functions
 **
 ** @author MADS Team 
 **
 ******************************************************************************/

/******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "hc32l110.h"
#include "ddl.h"
#include "uart.h"
#include "bt.h"
#include "gpio.h"
#include "clk.h"
#include "i2c.h"
#include "waveinit.h"
#include "epd.h"
#include "draw.h"

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                            
 ******************************************************************************/

/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')                                         
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')                                      *
 ******************************************************************************/

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                             
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
uint8_t cmd[3] = {0xAC,0x33,0x00};
uint8_t u8Recdata[8]={0x00};


static void uartPrintf(const char *format, ...) 
{
    char buffer[256]; // 缓冲区，用于存储格式化后的字符串
    va_list args;     // 可变参数列表
    uint8_t len = 0;
    uint8_t i = 0;

    // 初始化可变参数
    va_start(args, format);

    // 格式化字符串并存入 buffer 中
    len = vsnprintf(buffer, sizeof(buffer), format, args);

    // 清理可变参数列表
    va_end(args);

    // 如果格式化成功，逐字节发送字符串
    if (len > 0) {
        for (i = 0; i < len; i++) {
            // 调用 Uart_SendData 发送字符
            if (Uart_SendData(UARTCH1, (uint8_t)buffer[i]) != Ok) 
            {
                // 发送失败处理（根据实际需求添加）
                break;
            }
        }
    }
}

static void uartPrintfFloat(const char *head, const float data)
{
    uint8_t integerPart = (uint8_t)data;
    float decimalPart = data * 10000 - integerPart * 10000;
    uartPrintf("%s%d.%d !!\n",head,integerPart,(uint16_t)decimalPart);
}


static void uartInit(void)
{
    uint16_t timer=0;
    uint32_t pclk=0;

    stc_uart_config_t  stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;
    

    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);

    Gpio_InitIOExt(3,5,GpioDirOut,TRUE,FALSE,FALSE,FALSE);   
    Gpio_InitIOExt(3,6,GpioDirOut,TRUE,FALSE,FALSE,FALSE); 
    
    //通道端口配置
    Gpio_SetFunc_UART1TX_P35();
    Gpio_SetFunc_UART1RX_P36();
    //外设时钟使能
    Clk_SetPeripheralGate(ClkPeripheralBt,TRUE);//模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart1,TRUE);

    stcUartIrqCb.pfnRxIrqCb = NULL;
    stcUartIrqCb.pfnTxIrqCb = NULL;
    stcUartIrqCb.pfnRxErrIrqCb = NULL;
    stcConfig.pstcIrqCb = &stcUartIrqCb;
    stcConfig.bTouchNvic = FALSE;

    stcConfig.enRunMode = UartMode1;//测试项，更改此处来转换4种模式测试
    stcMulti.enMulti_mode = UartNormal;//测试项，更改此处来转换多主机模式，mode2/3才有多主机模式
    stcConfig.pstcMultiMode = &stcMulti;

    stcBaud.bDbaud = 1u;//双倍波特率功能
    stcBaud.u32Baud = 115200;//更新波特率位置
    stcBaud.u8Mode = UartMode1; //计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();
    timer=Uart_SetBaudRate(UARTCH1,pclk,&stcBaud);

    stcBtConfig.enMD = BtMode2;
    stcBtConfig.enCT = BtTimer;
    Bt_Init(TIM1, &stcBtConfig);//调用basetimer1设置函数产生波特率
    Bt_ARRSet(TIM1,timer);
    Bt_Cnt16Set(TIM1,timer);
    Bt_Run(TIM1);

    Uart_Init(UARTCH1, &stcConfig);
    Uart_EnableIrq(UARTCH1,UartRxIrq);
    Uart_ClrStatus(UARTCH1,UartRxFull);
    Uart_EnableFunc(UARTCH1,UartRx);
}

/**********************************************************
*  CRC校验类型：CRC8
*  多项式：X8+X5+X4+1
*  Poly:0011 0001 0x31
**********************************************************/

static uint8_t calcCrc8(unsigned char *message,unsigned char num)
{
    uint8_t i;
    uint8_t byte;
    uint8_t crc =0xFF;
    for (byte = 0;byte<num;byte++)
    {
        crc^=(message[byte]);
        for(i=8;i>0;--i)
        {
            if(crc&0x80)
                crc=(crc<<1)^0x31;
            else
                crc=(crc<<1);
        }
    }
    return crc;
}

en_result_t I2C_MasterWriteData(uint8_t *pu8Data,uint32_t u32Len)
{
    en_result_t enRet = Error;
    uint8_t u8i=0,u8State;
    I2C_SetFunc(I2cStart_En);

	while(1)
	{
		while(0 == I2C_GetIrq())
		{;}
		u8State = I2C_GetState();
		switch(u8State)
		{
			case 0x08:
			case 0x10:
				I2C_ClearFunc(I2cStart_En);
                I2C_WriteByte(0x70);//从设备地址发送
				break;
			case 0x18:
			case 0x28:	
				I2C_WriteByte(pu8Data[u8i++]);
				break;
			case 0x20:
			case 0x38:
				I2C_SetFunc(I2cStart_En);
				break;
			case 0x30:
				I2C_SetFunc(I2cStop_En);
				I2C_SetFunc(I2cStart_En);
				break;
			default:
				break;
		}			
		if(u8i>u32Len)
		{
			I2C_SetFunc(I2cStop_En);//此顺序不能调换，出停止条件
			I2C_ClearIrq();
			break;
		}
		I2C_ClearIrq();	
	}
    enRet = Ok;
    return enRet;
}

en_result_t I2C_MasterReadData(uint8_t *pu8Data,uint32_t u32Len)
{
    en_result_t enRet = Error;
    uint8_t u8i=0,u8State;
    
    I2C_SetFunc(I2cStart_En);
    
	while(1)
	{
		while(0 == I2C_GetIrq())
        {}
		u8State = I2C_GetState();
		switch(u8State)
		{
			case 0x08:
			case 0x10:
				I2C_ClearFunc(I2cStart_En);
				I2C_WriteByte(0x71);//从机地址发送OK
				break;
			case 0x40:
				if(u32Len>1)
				{
					I2C_SetFunc(I2cAck_En);
				}
				break;
			case 0x50:
				pu8Data[u8i++] = I2C_ReadByte();
				if(u8i==u32Len-1)
				{
					I2C_ClearFunc(I2cAck_En);
				}
				break;
			case 0x58:
				pu8Data[u8i++] = I2C_ReadByte();
				I2C_SetFunc(I2cStop_En);
				break;	
			case 0x38:
				I2C_SetFunc(I2cStart_En);
				break;
			case 0x48:
				I2C_SetFunc(I2cStop_En);
				I2C_SetFunc(I2cStart_En);
				break;
			default:
				I2C_SetFunc(I2cStart_En);//其他错误状态，重新发送起始条件
				break;
		}
		I2C_ClearIrq();
		if(u8i==u32Len)
		{
			break;
		}
	}
	enRet = Ok;
	return enRet;
}

static void i2cInit(void)
{
    stc_i2c_config_t stcI2cCfg;
    stc_clk_config_t stcCfg;
    DDL_ZERO_STRUCT(stcCfg);
    DDL_ZERO_STRUCT(stcI2cCfg);

    Clk_SetPeripheralGate(ClkPeripheralI2c,TRUE);
    
    stcI2cCfg.enFunc = I2cBaud_En;
    stcI2cCfg.u8Tm = 9;//300k=(24000000/(8*(9+1))
    stcI2cCfg.pfnI2cCb = NULL;
    stcI2cCfg.bTouchNvic = FALSE;

    I2C_DeInit(); 
    I2C_Init(&stcI2cCfg);
    Clk_SetPeripheralGate(ClkPeripheralGpio,TRUE);
    Gpio_SetIO(2, 5, TRUE);
    Gpio_SetIO(2, 6, TRUE);
    Gpio_InitIOExt(2,5,GpioDirOut,TRUE,FALSE,TRUE,FALSE); 
    Gpio_InitIOExt(2,6,GpioDirOut,TRUE,FALSE,TRUE,FALSE);
    
    Gpio_SetFunc_I2CDAT_P25() 
    Gpio_SetFunc_I2CCLK_P26()
    I2C_SetFunc(I2cMode_En);

}

static float temperatureConvert(uint32_t d0, uint32_t d1, uint32_t d2)
{
    float result = 0.0;
    uint32_t data = (d2 | (d1 << 8) | (((d0 & 0x0000000f)) << 16));
    result = (data / 1048576.0)*200.0 - 50.0;
    return result;
}

static float humidityConvert(uint32_t d0, uint32_t d1, uint32_t d2)
{
    float result = 0.0;
    uint32_t data = (((d2 & 0x000000f0) >> 4) | (d1 << 4) | (d0 << 12));
    result = (data / 1048576.0)*100.0;
    return result;
}




/**
 ******************************************************************************
 ** \brief  Main function of project
 **
 ** \return uint32_t return value, if needed
 **
 ******************************************************************************/
int32_t main(void)
{
    char data = 0;
    uint8_t crc = 0;
    float temperature = 0.0, humidity = 0.0;
    uartInit();
	  i2cInit();
    while (data != 0x36)
    {
        data = Uart_ReceiveData(UARTCH1);
        uartPrintf("Input 6 to start.\n");
        delay1ms(500);
    }
    data = 0;
    EPD_initWft0154cz17(FALSE);
		// spiInit();
		// initSsd1675();
    // Test_Display(0x00);
    // delay1ms(1000);
    // Test_Display(0xf0);
    // delay1ms(1000);
    // Test_Display(0x55);
    // delay1ms(1000);
    // Test_Display(0x00);
    // delay1ms(1000);
    // Test_Display(0xff);
    // delay1ms(1000);
    delay1ms(10000);
    // Test_Display(0x66);
    // delay1ms(15000);
    // Test_Display(0xff);
    // delay1ms(5000);

    // Test_Display2(0xff);
    // DRAW_initScreen();
    // DRAW_rectangle(10, 10, 50, 30, 1, 0); // 绘制黑色空心矩形
    // DRAW_hLine(0, 75, 152, 1);           // 绘制黑色水平线
    // DRAW_vLine(75, 0, 152, 1);           // 绘制黑色垂直线
    // DRAW_string(10, 20, "Hello", 0, BLACK);
    // DRAW_rotatedString(10, 10, "Hello111", 1, BLACK);
    // DRAW_rotatedString(10, 30, "Hello222", 2, BLACK);
    // DRAW_rotatedString(10, 60, "Hello333", 3, BLACK);

    // DRAW_string(10, 60, "Hello25", 2, BLACK);
    // DRAW_string(10, 80, "Hello36", 3, BLACK);
    // DRAW_char(10, 20, 'H', 1, BLACK);
    // DRAW_char(10, 40, 'H', 2, BLACK);
    // DRAW_char(10, 60, 'H', 3, BLACK);

    // DRAW_char(70, 20, 'A', 1, BLACK);
    // DRAW_char(70, 40, 'A', 2, BLACK);
    // DRAW_char(70, 60, 'A', 3, BLACK);
    // DRAW_rotateScreen90();
    // DRAW_pixel(0, 0, WHITE);
    // DRAW_pixel(1, 1, WHITE);
    // DRAW_pixel(2, 2, WHITE);
    // DRAW_pixel(3, 3, WHITE);
    // DRAW_pixel(4, 4, WHITE);
    // DRAW_pixel(10, 10, WHITE);
    // DRAW_pixel(20, 20, WHITE);
    // DRAW_pixel(30, 30, WHITE);
    // DRAW_pixel(40, 40, WHITE);
    // DRAW_pixel(50, 50, WHITE);
    // DRAW_pixel(100, 100, WHITE);
    // DRAW_hLine(10, 10, 50, WHITE);           // 绘制黑色水平线
    //     I2C_MasterWriteData(&cmd[0],3);
    //     delay1ms(80);
    //     I2C_MasterReadData(&u8Recdata[0],7);


    //     temperature = temperatureConvert((uint32_t)u8Recdata[3],(uint32_t)u8Recdata[4],(uint32_t)u8Recdata[5]);
    //     humidity = humidityConvert((uint32_t)u8Recdata[1],(uint32_t)u8Recdata[2],(uint32_t)u8Recdata[3]);


    // DRAW_DisplayTempHumiRot(temperature, humidity);

    // DRAW_Char40_Rot(10, 10, '0', BLACK);
    // DRAW_Char40_Rot(30, 10, '1', BLACK);
    // DRAW_Char40_Rot(50, 10, '2', BLACK);
    // DRAW_Char40_Rot(70, 10, '3', BLACK);
    // DRAW_Char40_Rot(90, 10, '4', BLACK);
    // DRAW_Char40_Rot(10, 60, '5', BLACK);
    // DRAW_Char40_Rot(30, 60, '6', BLACK);
    // DRAW_Char40_Rot(50, 60, '7', BLACK);
    // DRAW_Char40_Rot(70, 60, '8', BLACK);
    // DRAW_Char40_Rot(90, 60, '9', BLACK);


    // DRAW_Char40_Rot(10, 10, '.', BLACK);
    // DRAW_Char40_Rot(30, 10, '°', BLACK);
    // DRAW_Char40_Rot(50, 10, '%', BLACK);
    // DRAW_Char40_Rot(10, 60, ':', BLACK);
    // DRAW_Char40_Rot(30, 60, '-', BLACK);
    // DRAW_Char40_Rot(50, 60, 'C', BLACK);

    // DRAW_outputScreen();

    // delay1ms(10000);

    // EPD_poweroff();

    while(data != 0x39)
    {
        I2C_MasterWriteData(&cmd[0],3);
        delay1ms(80);
        I2C_MasterReadData(&u8Recdata[0],7);


        temperature = temperatureConvert((uint32_t)u8Recdata[3],(uint32_t)u8Recdata[4],(uint32_t)u8Recdata[5]);
        humidity = humidityConvert((uint32_t)u8Recdata[1],(uint32_t)u8Recdata[2],(uint32_t)u8Recdata[3]);

        uartPrintfFloat("Temperature is ",temperature);
        uartPrintfFloat("Humidity is ",humidity);
        crc = calcCrc8(&u8Recdata[0],6);
        uartPrintf("CRC = %x !!\n",crc);

        uartPrintf("D0 = %x !!\n",u8Recdata[0]);
        // uartPrintf("D1 = %x !!\n",u8Recdata[1]);
        // uartPrintf("D2 = %x !!\n",u8Recdata[2]);
        // uartPrintf("D3 = %x !!\n",u8Recdata[3]);
        // uartPrintf("D4 = %x !!\n",u8Recdata[4]);
        // uartPrintf("D5 = %x !!\n",u8Recdata[5]);
        uartPrintf("D6 = %x !!\n",u8Recdata[6]);
        DRAW_initScreen();
        DRAW_DisplayTempHumiRot(temperature, humidity);
        DRAW_outputScreen();
        data = Uart_ReceiveData(UARTCH1);
        uartPrintf("Input 9 to exit.\n");

        delay1ms(10000);

        // data = Uart_ReceiveData(UARTCH1);
        // if (data != 0)
        // {
        //     uartPrintf("Input is: %c\n",data);
        //     data = 0;
        // }

    }
    EPD_poweroff();
    while(1)
    {}

}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


