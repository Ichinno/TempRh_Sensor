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
#include "lpt.h"
#include "lpm.h"

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
static volatile boolean_t wakeup = FALSE;
static volatile boolean_t tg8s = FALSE;
// static volatile boolean_t tg2s = FALSE;
static float temperature = 0.0, humidity = 0.0;
static boolean_t linkFlag = FALSE;

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

void LptInt(void)
{
    if (TRUE == Lpt_GetIntFlag())
    {
        Lpt_ClearIntFlag();
        wakeup = TRUE;
        if (timer0 < 4)  // 10s
        {
            timer0++;
        }
        else
        {
            timer0 = 0;
            tg8s = TRUE;
        }
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
    
    stcConfig.pfnTim1Cb = NULL;
        
    stcConfig.enGateP = BtPositive;
    stcConfig.enGate  = BtGateDisable;
    stcConfig.enPRS   = BtPCLKDiv8;
    stcConfig.enTog   = BtTogDisable;
    stcConfig.enCT    = BtTimer;
    stcConfig.enMD    = BtMode2;
    //Bt初始化
    Bt_Stop(TIM0);

    Bt_Init(TIM0, &stcConfig);
    
    //TIM1中断使能
    Bt_ClearIntFlag(TIM0);
    Bt_EnableIrq(TIM0);
    EnableNvic(TIM0_IRQn, 0, TRUE);

    //设置重载值和计数值，启动计数
    Bt_ARRSet(TIM0, 0xC537);
    Bt_Cnt16Set(TIM0, 0xC537);
    Bt_Run(TIM0);

}

static void lpmInit(void)
{
    stc_lpt_config_t stcConfig;
    stc_lpm_config_t stcLpmCfg;
    uint16_t         u16ArrData = 0;

    Clk_Enable(ClkRCL, TRUE);
    //使能Lpt、GPIO外设时钟
    Clk_SetPeripheralGate(ClkPeripheralLpTim, TRUE);

    stcConfig.enGateP  = LptPositive;
    stcConfig.enGate   = LptGateDisable;
    stcConfig.enTckSel = LptIRC32K;
    stcConfig.enTog    = LptTogDisable;
    stcConfig.enCT     = LptTimer;
    stcConfig.enMD     = LptMode2;
    
    stcConfig.pfnLpTimCb = LptInt;
    Lpt_Init(&stcConfig);
    //Lpm Cfg
    stcLpmCfg.enSEVONPEND   = SevPndDisable;
    stcLpmCfg.enSLEEPDEEP   = SlpDpEnable;
    stcLpmCfg.enSLEEPONEXIT = SlpExtDisable;
    Lpm_Config(&stcLpmCfg);
    
    //Lpt 中断使能
    Lpt_ClearIntFlag();
    Lpt_EnableIrq();
    EnableNvic(LPTIM_IRQn, 0, TRUE);
    
    
    //设置重载值，计数初值，启动计数
    Lpt_ARRSet(u16ArrData);
    Lpt_Run();

    // 进入低功耗模式……
    Lpm_GotoLpmMode(); 

}

static void task3(void)
{
    if (tg1) // 10ms
    {
        tg1 = FALSE;
        UARTIF_passThrough();
        (void)E104_getLinkState();
        (void)E104_getDataState();
        E104_executeCommand();
    }
}

static void task1(void)
{
    DRAW_initScreen();
    DRAW_DisplayTempHumiRot(temperature,humidity,linkFlag);

    DRAW_outputScreen();
}

static void task0(void)
{
    I2C_MasterWriteData(&cmd[0],3);
    delay1ms(80);
    I2C_MasterReadData(&u8Recdata[0],7);

    temperature = temperatureConvert((uint32_t)u8Recdata[3],(uint32_t)u8Recdata[4],(uint32_t)u8Recdata[5]);
    humidity = humidityConvert((uint32_t)u8Recdata[1],(uint32_t)u8Recdata[2],(uint32_t)u8Recdata[3]);
    if (E104_getLinkState())
    {
        UARTIF_uartPrintfFloat("Temperature is ",temperature);
        UARTIF_uartPrintfFloat("Humidity is ",humidity);
        if (!linkFlag)
        {
            linkFlag = TRUE;
        }
    }
    else
    {
        if (linkFlag)
        {
            linkFlag = FALSE;
            E104_setWakeUpMode();

            delay1ms(30);
            E104_setSleepMode();
        }

    }
}

static void handleClearEpdEvent(void)
{
    typedef enum 
    {
        STATE_IDLE,     // 空闲状态
        STATE_WAITING_3_4,  // 等待第三次唤醒
        STATE_WAITING_4,  // 等待第四次唤醒
    } State;

    static State currentState = STATE_IDLE;
    static uint8_t counter = 0;
    // static boolean_t firstflag = TRUE;

    switch (currentState)
    {
        case STATE_IDLE:
            if (counter > 40 && tg8s)
            {
                currentState = STATE_WAITING_3_4;
                counter = 0;
            }
            else
            {
                counter++; 
            }
            break;
        case STATE_WAITING_3_4:
            if (counter < 2)
            {
                counter++;
            }
            else
            {
                EPD_poweroff();
                delay1ms(10);
                EPD_initWft0154cz17(TRUE);
                currentState = STATE_WAITING_4;
                counter = 0;
            }
            break;
        case STATE_WAITING_4:
            if (counter < 3)
            {
                counter++;
            }
            else
            {
                EPD_initWft0154cz17(FALSE);
                currentState = STATE_IDLE;
                counter = 0;
            }
            break;
        default:
            break;
    }
    // UARTIF_uartPrintf(0, "State is %d, counter is %d.\n",currentState,counter);
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
//    uint8_t data = 0;
//   uint8_t crc = 0;
//    boolean_t trig1s = FALSE;
    UARTIF_uartInit();
    i2cInit();
    UARTIF_lpuartInit();
    E104_ioInit();
    delay1ms(30);
    E104_setSleepMode();

    // while (data != 0x36)
    // {
    //     UARTIF_uartPrintf(0, "Input 6 to start.\n");
    //     data = Uart_ReceiveData(UARTCH1);
    //     delay1ms(500);
    // }
    // timInit();
    EPD_initWft0154cz17(TRUE);
    task0();
    task1();
    lpmInit();

    while(1)
    {
        if (wakeup)
        {
            wakeup = FALSE;
            task0();
            // UARTIF_uartPrintf(0, "Wake up! \n");
            // handleClearEpdEvent();
            if (tg8s)
            {
                tg8s = FALSE;
                task1();
                // UARTIF_uartPrintf(0, "tg8s! \n");
            }
            Lpm_GotoLpmMode(); 
        }
    }
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


