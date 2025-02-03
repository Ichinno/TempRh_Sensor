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
#include "lpuart.h"
#include "queue.h"
#include "uart_interface.h"
#include "e104.h"
#include "image.h"

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
// static uint8_t uart1RxFlg = 0;
static volatile uint16_t timer0 = 0;
static volatile boolean_t tg1 = FALSE;

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                             
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
uint8_t cmd[3] = {0xAC,0x33,0x00};
uint8_t u8Recdata[8]={0x00};

void Bt0Int(void)
{
    /* 5ms */
    // if (TRUE == Bt_GetIntFlag(TIM0))
    // {
        Bt_ClearIntFlag(TIM0);
    // }
    // timer0++;
    if (timer0 < 199)  // 1s
    {
        timer0++;
    }
    else
    {
        timer0 = 0;
    }

    if ((timer0 % 2) == 0) // 10ms
    {
        tg1 = TRUE;
    }
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

static void timInit(void)
{
    stc_bt_config_t   stcConfig;
    en_result_t       enResult = Error;
    // uint16_t          u16ArrData = 0xC567;
    // uint16_t          u16InitCntData = 0xC567;
    
    stcConfig.pfnTim1Cb = NULL;
        
    stcConfig.enGateP = BtPositive;
    stcConfig.enGate  = BtGateDisable;
    stcConfig.enPRS   = BtPCLKDiv8;
    stcConfig.enTog   = BtTogDisable;
    stcConfig.enCT    = BtTimer;
    stcConfig.enMD    = BtMode2;
    //Bt初始化
    Bt_Stop(TIM0);

    enResult = Bt_Init(TIM0, &stcConfig);
    
    //TIM1中断使能
    Bt_ClearIntFlag(TIM0);
    Bt_EnableIrq(TIM0);
    EnableNvic(TIM0_IRQn, 0, TRUE);

    //设置重载值和计数值，启动计数
    Bt_ARRSet(TIM0, 0xC537);
    Bt_Cnt16Set(TIM0, 0xC537);
    Bt_Run(TIM0);

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
   uint8_t data = 0;
//   uint8_t crc = 0;
//    boolean_t trig1s = FALSE;
   float temperature = 0.0, humidity = 0.0;
    UARTIF_uartInit();
    i2cInit();
    UARTIF_lpuartInit();

    while (data != 0x36)
    {
        UARTIF_uartPrintf(0, "Input 6 to start.\n");
        data = Uart_ReceiveData(UARTCH1);
        delay1ms(500);
    }
    // E104_ioInit();
    // timInit();

    data = 0;
    EPD_initWft0154cz17(TRUE);
    // delay1ms(5000);

    // uartPrintf(2, "0");
//    LPUart_SendData(0x00);
//    delay1ms(2);

    // while (data == 0)
    // {
    //     uartPrintf(2, "AT");
    //     uartPrintf(0, "Sent AT to E104-BT01, now try to get receive data.\n");
    //     delay1ms(10);
    //     data = LPUart_ReceiveData();
    // }


    while(data != 0x39)
    {
        
    //     uartPrintf(0, "Receive data is %c.\n",data);
    //     data = LPUart_ReceiveData();
    // }

    // while(data != 0x39)
    // {
        I2C_MasterWriteData(&cmd[0],3);
        delay1ms(80);
        I2C_MasterReadData(&u8Recdata[0],7);

        temperature = temperatureConvert((uint32_t)u8Recdata[3],(uint32_t)u8Recdata[4],(uint32_t)u8Recdata[5]);
        humidity = humidityConvert((uint32_t)u8Recdata[1],(uint32_t)u8Recdata[2],(uint32_t)u8Recdata[3]);

        UARTIF_uartPrintfFloat("Temperature is ",temperature);
        UARTIF_uartPrintfFloat("Humidity is ",humidity);
    //     crc = calcCrc8(&u8Recdata[0],6);
    //     uartPrintf(0, "CRC = %x !!\n",crc);
    //     uartPrintf(0, "D0 = %x !!\n",u8Recdata[0]);
    //     uartPrintf(0, "D6 = %x !!\n",u8Recdata[6]);

        DRAW_initScreen();
        DRAW_DisplayTempHumiRot(temperature,humidity);

        DRAW_outputScreen();

        data = Uart_ReceiveData(UARTCH1);
        UARTIF_uartPrintf(0,"Input 9 to exit.\n");
        delay1ms(8000);

    }
    EPD_poweroff();
//     while(1)
//     {
//         if (tg1) // 10ms
//         {
//             tg1 = FALSE;
//             UARTIF_passThrough();
//             (void)E104_getLinkState();
//             (void)E104_getDataState();
//             E104_executeCommand();

//         }
//     }
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


