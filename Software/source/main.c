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
#include "spi.h"
//#include "image.h"

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                            
 ******************************************************************************/
#define DC_H    Gpio_SetIO(0, 3, 1) //DC输出高
#define DC_L    Gpio_SetIO(0, 3, 0) //DC输出低
#define RST_H   Gpio_SetIO(2, 3, 1) //RST输出高
#define RST_L   Gpio_SetIO(2, 3, 0) //RST输出低
// 测试图
#define PIC_WHITE                   255  // 全白
#define PIC_BLACK                   254  // 全黑
#define PIC_Orientation             253  // 方向图
#define PIC_CHESSBOARD              252  // 单像素棋盘格
#define PIC_Source_Line             251  // Source线
#define PIC_Gate_Line               250  // Gate线
#define PIC_LEFT_BLACK_RIGHT_WHITE  249  // 左黑右白
#define PIC_UP_BLACK_DOWN_WHITE     248  // 上黑下白

// 用户自定义Demo图
#define PIC_01                        1
#define PIC_02                        2
#define PIC_03                        3
#define PIC_04                        4
#define PIC_05                        5
#define PIC_06                        6


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

static void spiWriteCmd(uint8_t cmd)
{
    DC_L;
    Spi_SetCS(TRUE);
    Spi_SetCS(FALSE);
    Spi_SendData(cmd);
    Spi_SetCS(TRUE);
}

static void spiWriteData(uint8_t data)
{
    DC_H;
    Spi_SetCS(TRUE);
    Spi_SetCS(FALSE);
    Spi_SendData(data);
    Spi_SetCS(TRUE);
    DC_L;
}

static void spiInit(void)
{
	  stc_spi_config_t  SPIConfig;

    Clk_SetPeripheralGate(ClkPeripheralSpi,TRUE); //SPI外设时钟打开
    Gpio_InitIO(2, 3, GpioDirOut);
    Gpio_SetIO(2, 3, 1);               //RST输出高
    Gpio_InitIO(0, 3, GpioDirOut);
    Gpio_SetIO(0, 3, 0);               //DC输出高

    Gpio_SetFunc_SPICS_P14();
    Gpio_SetFunc_SPIMOSI_P24();
    Gpio_SetFunc_SPICLK_P15();
    
    Spi_SetCS(TRUE);
    //配置SPI
    SPIConfig.bCPHA = Spicphafirst;
    SPIConfig.bCPOL = Spicpollow;
    SPIConfig.bIrqEn = FALSE;
    SPIConfig.bMasterMode = SpiMaster;
    SPIConfig.u8BaudRate = SpiClkDiv2;
    SPIConfig.pfnIrqCb = NULL;

    Spi_Init(&SPIConfig);

}

static void resetSsd1675(void)
{
    Spi_SetCS(TRUE);
    DC_L;
    RST_H;
    delay1ms(100);                
    RST_L;
    delay1ms(10);                
    RST_H;
    delay1ms(5000);
    spiWriteCmd(0x12);   //  
    delay1ms(5000);

}

// 写波形数据表 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void writeLut()
{
  unsigned char i;
  
  spiWriteCmd(0x32);   // write LUT register
  for(i=0;i<70;i++)       // write LUT register 20160527
    spiWriteData(init_data[i]);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xx   图片显示函数    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void DIS_IMG(unsigned char num)
{
  unsigned int row, col;
  unsigned int pcnt;

  spiWriteCmd(0x4E);   // set RAM x address count to 0;
  spiWriteData(0x00);
  spiWriteCmd(0x4F);   // set RAM y address count to 151;
  spiWriteData(0x97);
  spiWriteData(0x00);
        
  spiWriteCmd(0x24);   //
  pcnt = 0;                     // 复位或保存提示字节序号
  for(col=0; col<152; col++)    // 152 GATE
  {
    for(row=0; row<19; row++)   // 总共128 SOURCE，每个像素1bit,即 152/8=19 字节
    {
      switch (num)
      {
        case PIC_WHITE:
          spiWriteData(0xff);
          break;  

        case PIC_BLACK:
          spiWriteData(0x00);
          break;  
          
        case PIC_CHESSBOARD:
          if(col%2)
              spiWriteData(0xaa);  // 奇数Gate行
          else
              spiWriteData(0x55);  // 偶数Gate行
          break;  
                                        
        case PIC_Source_Line:
          spiWriteData(0xaa);
          break;  
                                        
        case PIC_Gate_Line:
          if(col%2)
              spiWriteData(0xff);  // 奇数Gate行
          else
              spiWriteData(0x00);  // 偶数Gate行
          break;  
                                        
        case PIC_LEFT_BLACK_RIGHT_WHITE:
          if(col>=76)
            spiWriteData(0xff);
          else
            spiWriteData(0x00);
          break;
                                        
        case PIC_UP_BLACK_DOWN_WHITE:
          if(row>9)
            spiWriteData(0xff);
          else if(row==9)
            spiWriteData(0x0f);
          else
            spiWriteData(0x00);
          break;  
                                        
        // case PIC_01:
        //   spiWriteData(gImage_01[pcnt]);
        //   break;

        // case PIC_02:
        //   spiWriteData(gImage_02[pcnt]);
        //   break;

        // case PIC_03:
        //   spiWriteData(gImage_03[pcnt]);
        //   break;

        // case PIC_04:
        //   spiWriteData(gImage_04[pcnt]);
        //   break;

        // case PIC_05:
        //   spiWriteData(gImage_05[pcnt]);
        //   break;

        // case PIC_06:
        //   spiWriteData(gImage_06[pcnt]);
        //   break;

        default:
          break;
        }
      pcnt++;
    }
  }
  spiWriteCmd(0x20);
  delay1ms(5000);
}

// 电子纸驱动初始化 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
static void initSsd1675()
{
  resetSsd1675();                      // 电子纸控制器复位
  spiWriteCmd(0x74);       // Set Analog Block Control
    spiWriteData(0x54);    // A[8:0]=
  spiWriteCmd(0x7E);       // Set Analog Block Control
    spiWriteData(0x3B);    // A[8:0]=
  spiWriteCmd(0x01);       // Gate Setting
    spiWriteData(0x97);    // A[8:0]=0x0097(Set Mux for 151（0x0097）+1=152)
    spiWriteData(0x00);    // A[8:0]=0x0097(Set Mux for 151（0x0097）+1=152)
    spiWriteData(0x00);    // B[2]:GD=0[POR](G0 is the 1st gate output channel)  B[1]:SM=0[POR](left and right gate interlaced)  B[0]:TB=0[POR](scan from G0 to G319)

  spiWriteCmd(0x0C);       // Booster Soft Start Setting
    spiWriteData(0x8B);    // 0x8B
    spiWriteData(0x9C);    // 0x9C
//    spiWriteData(0x86);    // 0x86 for str 1            
//    spiWriteData(0x96);    // 0x96[POR] for str 2,  3.0V电源、30V压差、点图VGH从 V下降到 V；2.0V电源、30V压差、点图VGH从 V下降到 V；         
    spiWriteData(0xA6);    // 0xA6 for str 3   2.0V电源、30V压差、点图VGH从 V下降到 V，，就选这个         
//    spiWriteData(0xB6);    // 0xB6 for str 4     
//    spiWriteData(0xC6);    // 0xC6 for str 5            
//    spiWriteData(0xD6);    // 0xD6 for str 6            
//    spiWriteData(0xE6);    // 0xE6 for str 7            
//    spiWriteData(0xF6);    // 0xF6 for str 8       
    spiWriteData(0x0F);    // Set Digital Block Control


  spiWriteCmd(0x03);       // Gate Driving voltage
    spiWriteData(0x19);    // A[4:0] = 19h [POR] , VGH at 21V [POR]  实测VGH=+21.1V~+21.4V，VGL=-21.0V


  spiWriteCmd(0x04);       // Source Driving voltage
    spiWriteData(0x41);    // A[7:0] = 41h [POR], VSH1 at 15V
//    spiWriteData(0x3C);    // A[7:0] = 3Ch [POR], VSH1 at 14V
//    spiWriteData(0x37);    // A[7:0] = 37h [POR], VSH1 at 13V
//    spiWriteData(0x32);    // A[7:0] = 32h [POR], VSH1 at 12V
//    spiWriteData(0x2D);    // A[7:0] = 2Dh [POR], VSH1 at 11V
//    spiWriteData(0x28);    // A[7:0] = 28h [POR], VSH1 at 10V
//    spiWriteData(0x23);    // A[7:0] = 23h [POR], VSH1 at  9V
//    spiWriteData(0x22);    // A[7:0] = 22h [POR], VSH1 at  8.8V
//    spiWriteData(0xC6);    // A[7:0] = C6h [POR], VSH1 at  8V
//    spiWriteData(0xBC);    // A[7:0] = BCh [POR], VSH1 at  7V
//    spiWriteData(0xB2);    // A[7:0] = B2h [POR], VSH1 at  6V
//    spiWriteData(0xA8);    // A[7:0] = A8h [POR], VSH1 at  5V
//    spiWriteData(0x9E);    // A[7:0] = C6h [POR], VSH1 at  4V
//    spiWriteData(0x94);    // A[7:0] = C6h [POR], VSH1 at  3V

    spiWriteData(0xA8);    // B[7:0] = A8h [POR], VSH2 at  5V

    spiWriteData(0x32);    // C[7:0] = 32h [POR], VSL at -15V
//    spiWriteData(0x2E);    // C[7:0] = 2Eh [POR], VSL at -14V
//    spiWriteData(0x2A);    // C[7:0] = 2Ah [POR], VSL at -13V
//    spiWriteData(0x26);    // C[7:0] = 26h [POR], VSL at -12V
//    spiWriteData(0x22);    // C[7:0] = 22h [POR], VSL at -11V
//    spiWriteData(0x1E);    // C[7:0] = 1Eh [POR], VSL at -10V
//    spiWriteData(0x1A);    // C[7:0] = 1Ah [POR], VSL at  -9V
//    spiWriteData(0x19);    // C[7:0] = 19h [POR], VSL at  -8.8V
//    spiWriteData(0x16);    // C[7:0] = 16h [POR], VSL at  -8V
//    spiWriteData(0x12);    // C[7:0] = 12h [POR], VSL at  -7V
//    spiWriteData(0x0E);    // C[7:0] = 0Eh [POR], VSL at  -6V
//    spiWriteData(0x0A);    // C[7:0] = 0Ah [POR], VSL at  -5V
//    spiWriteData(0x06);    // C[7:0] = 06h [POR], VSL at  -4V
//    spiWriteData(0x02);    // C[7:0] = 06h [POR], VSL at  -3V


  spiWriteCmd(0x3A);       // number of dummy line period   set dummy line for 50Hz frame freq
    spiWriteData(0x11);    // A[6:0]=0(Number of dummy line period in term of TGate)  
  spiWriteCmd(0x3B);       // Gate line width   set gate line for 50Hz frame freq
    spiWriteData(0x0D);    // A[3:0]=0DH(118us)  Line width in us   118us*(152+17)=20001us=19.942ms      
//    spiWriteData(0x0A);    // A[3:0]=1010(59us)  Line width in us   59us*(152+43)=20001us=20.001ms      
  spiWriteCmd(0x3C);       // board  注意：与之前的IC不一样
//    spiWriteData(0x00);    // GS0-->GS0 LUT0
//    spiWriteData(0x01);    // GS0-->GS1 LUT1
//    spiWriteData(0x02);    // GS1-->GS0 LUT2
    spiWriteData(0x03);    // GS1-->GS1 LUT3  开机第一次刷新Border从白到白
//    spiWriteData(0xC0);    // VBD-->HiZ  后面刷新时Border都是高阻

  spiWriteCmd(0x11);       // data enter mode
    spiWriteData(0x01);    // 01 –Y decrement, X increment,
  spiWriteCmd(0x44);       // set RAM x address start/end, in page 35
    spiWriteData(0x00);    // RAM x address start at 00h;
    spiWriteData(0x12);    // RAM x address end at 12h(18+1)*8->152 
  spiWriteCmd(0x45);       // set RAM y address start/end, in page 35
    spiWriteData(0x27);    // RAM y address start at 0127h;
    spiWriteData(0x01);    // RAM y address start at 0127h;
    spiWriteData(0x00);    // RAM y address end at 00h;
    spiWriteData(0x00);    // 高位地址=0

  spiWriteCmd(0x2C);     // vcom   注意：与之前的IC不一样 范围：-0.2~-3V
//    spiWriteData(0x78);    //-3V
//    spiWriteData(0x64);    //-2.5V
//    spiWriteData(0x60);    //-2.4V 
//    spiWriteData(0x58);    //-2.2V 
    spiWriteData(0x54);    //-2.1V 
//    spiWriteData(0x50);    //-2V
//    spiWriteData(0x3C);    //-1.5V
//    spiWriteData(0x34);    //-1.3V
//    spiWriteData(0x28);    //-1V 
//    spiWriteData(0x14);    //-0.5V

  spiWriteCmd(0x37);       // 打开前后两幅图对比模式
    spiWriteData(0x00);    // 
    spiWriteData(0x00);    // 
    spiWriteData(0x00);    // 
    spiWriteData(0x00);    // 
    spiWriteData(0x80);    // 
  writeLut();
  spiWriteCmd(0x21);       // 不管前一幅画面 Option for Display Update  注意：与之前的IC不一样      
    //spiWriteData(0x40);    // A[7:4] Red RAM option   A[3:0] BW RAM option  0000:Normal  0100:Bypass RAM content as 0(数据置0)    0101:Bypass RAM content as 1(数据置1)  1000:Inverse RAM content数据0、1互换
      spiWriteData(0x50);    // A[7:4] Red RAM option   A[3:0] BW RAM option  0000:Normal  0100:Bypass RAM content as 0(数据置0)    0101:Bypass RAM content as 1(数据置1)  1000:Inverse RAM content数据0、1互换
  spiWriteCmd(0x22);
    //spiWriteData(0xCB);    // (Enable Clock Signal, Enable CP) (INITIAL DISPLAY,Disable CP,Disable Clock Signal)
    spiWriteData(0xC7);    // (Enable Clock Signal, Enable CP) (Display update,Disable CP,Disable Clock Signal) 正常更新顺序
  DIS_IMG(PIC_BLACK);         // INITIAL DISPLAY+全白到全白 清屏，这样可防止开机出现花屏的问题

  spiWriteCmd(0x21);       // Option for Display Update
    spiWriteData(0x00);    // Normal  后面刷新恢复正常的黑白红显示
  spiWriteCmd(0x22);
    spiWriteData(0xC7);    // (Enable Clock Signal, Enable CP) (Display update,Disable CP,Disable Clock Signal) 正常更新顺序
//    spiWriteData(0xF7);    // (Enable Clock Signal, Enable CP, Load Temperature value, Load LUT) (Display update,Disable CP,Disable Clock Signal)
  spiWriteCmd(0x3C);       // board
    spiWriteData(0xC0);    // VBD-->HiZ  后面刷新时Border都是高阻
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

		spiInit();
		initSsd1675();
    while(1)
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

        // data = Uart_ReceiveData(UARTCH1);
        // if (data != 0)
        // {
        //     uartPrintf("Input is: %c\n",data);
        //     data = 0;
        // }

        delay1ms(1000);
    }

}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


