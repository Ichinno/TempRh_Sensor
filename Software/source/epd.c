/******************************************************************************
 * Copyright (C) 2021, 
 *
 *  
 *  
 *  
 *  
 *
 ******************************************************************************/
 
/******************************************************************************
 ** @file epd.c
 **
 ** @brief Source file for epd functions
 **
 ** @author MADS Team 
 **
 ******************************************************************************/

/******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "gpio.h"
#include "spi.h"
#include "epd.h"
/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                            
 ******************************************************************************/
#define DC_H    Gpio_SetIO(0, 3, 1) //DC输出高
#define DC_L    Gpio_SetIO(0, 3, 0) //DC输出低
#define RST_H   Gpio_SetIO(2, 3, 1) //RST输出高
#define RST_L   Gpio_SetIO(2, 3, 0) //RST输出低

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
// Following is full mode
// const unsigned char lut_vcomDC_WFT0290CZ10[42] =
// {
// 0x00 ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
// 0x60 ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
// 0x00 ,0x14 ,0x00 ,0x00 ,0x00 ,0x01,
// 0x00 ,0x13 ,0x0A ,0x01 ,0x00 ,0x01,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// };

// const unsigned char lut_ww_WFT0290CZ10[42] =
// {
// 0x40 ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
// 0x90 ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
// 0x10 ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
// 0xA0 ,0x13 ,0x01 ,0x00 ,0x00 ,0x01,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// };

// const unsigned char lut_bw_WFT0290CZ10[42] =
// {
// 0x40 ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
// 0x90 ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
// 0x00 ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
// 0x99 ,0x0C ,0x01 ,0x03 ,0x04 ,0x01,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// };

// const unsigned char lut_wb_WFT0290CZ10[42] =
// {
// 0x40 ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
// 0x90 ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
// 0x00 ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
// 0x99 ,0x0B ,0x04 ,0x04 ,0x01 ,0x01,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// };

// const unsigned char lut_bb_WFT0290CZ10[42] =
// {
// 0x80 ,0x0A ,0x00 ,0x00 ,0x00 ,0x01,
// 0x90 ,0x14 ,0x14 ,0x00 ,0x00 ,0x01,
// 0x20 ,0x14 ,0x0A ,0x00 ,0x00 ,0x01,
// 0x50 ,0x13 ,0x01 ,0x00 ,0x00 ,0x01,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
// };


//  following is the part mode

const unsigned char lut_vcomDC_WFT0290CZ10[44] =
{
  0x00, 0x20, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
};

const unsigned char lut_ww_WFT0290CZ10[42] =
{
  0x00, 0x20, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_bw_WFT0290CZ10[42] =
{
  0x80, 0x20, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_wb_WFT0290CZ10[42] =
{
  0x40, 0x20, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_bb_WFT0290CZ10[42] =
{
  0x00, 0x20, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                             
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
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

static void WRITE_LUT(unsigned char COM,const unsigned char *lut, const unsigned char size)
{
    unsigned char i;
    spiWriteCmd(COM);   // write LUT register
    for(i = 0; i < size; i++) {
    spiWriteData(lut[i]);
    }
}

static void resetWft0154cz17(void)
{
    Spi_SetCS(TRUE);
    DC_L;
    RST_H;
    delay1ms(100);                
    RST_L;
    delay1ms(2);                
    RST_H;
    delay1ms(100);
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

void EPD_initWft0154cz17(boolean_t isfull)
{
    int i = 0,j = 0;

    spiInit();
    resetWft0154cz17();                      // 电子纸控制器复位


    spiWriteCmd(0x01);
    spiWriteData(0x03);
    spiWriteData(0x00);
    spiWriteData(0x2b);
    spiWriteData(0x2b);
    spiWriteData(0x03);

    spiWriteCmd(0x06); //boost soft start
    spiWriteData(0x17);
    spiWriteData(0x17);
    spiWriteData(0x17);

    spiWriteCmd(0x00); //panel setting
    if (isfull)
    {
        spiWriteData(0xdf); //LUT from OTP��160x296
    }
    else
    {
        spiWriteData(0xff); //LUT from OTP��160x296
    }

    spiWriteData(0x0d); //VCOM to 0V fast

    spiWriteCmd(0x30); //
    spiWriteData(0x3a); //

    spiWriteCmd(0x61); //resolution setting
    spiWriteData(0x98); //152
    spiWriteData(0x00); //152
    spiWriteData(0x98);
  

    spiWriteCmd(0x82); //vcom_DC setting
    spiWriteData(0x08); 
    spiWriteCmd(0X50); //VCOM AND DATA INTERVAL SETTING
    spiWriteData(0x17); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7



    if (!isfull)
    {
        WRITE_LUT(0x20, lut_vcomDC_WFT0290CZ10,44);
        WRITE_LUT(0x21, lut_ww_WFT0290CZ10,42);
        WRITE_LUT(0x22, lut_bw_WFT0290CZ10,42);
        WRITE_LUT(0x23, lut_wb_WFT0290CZ10,42);
        WRITE_LUT(0x24, lut_bb_WFT0290CZ10,42);
    }
    spiWriteCmd(0x04);
    delay1ms(2000);

    spiWriteCmd(0x13);
    delay1ms(2);

    DC_H;
    Spi_SetCS(TRUE);
    Spi_SetCS(FALSE);
    // Spi_SendData(data);

    for (i = 0; i < 152; i++)
    {
        for (j = 0; j < 19; j++)
        {
            Spi_SendData(0xff);
        }
    }

    Spi_SetCS(TRUE);
    DC_L;
    delay1ms(2);
    spiWriteCmd(0x12);
}

void Test_Display(unsigned char data)
{
    int i = 0,j = 0;
    spiWriteCmd(0x13);
    delay1ms(2);

    DC_H;
    Spi_SetCS(TRUE);
    Spi_SetCS(FALSE);
    for (i = 0; i < 152; i++)
    {
        for (j = 0; j < 19; j++)
        {
            Spi_SendData(data);
        }
    }

    Spi_SetCS(TRUE);
    DC_L;
    delay1ms(2);

    spiWriteCmd(0x12);
} 

void Test_Display2(unsigned char data)
{
    int i = 0,j = 0;
//    unsigned char counter = 0;
    spiWriteCmd(0x13);
    delay1ms(2);

    DC_H;
    Spi_SetCS(TRUE);
    Spi_SetCS(FALSE);

    for (i = 0; i < 130; i++)
    {
        for (j = 0; j < 19; j++)
        {
            Spi_SendData(data);
        }
    }

    for (; i < 152; i++)
    {

            Spi_SendData(0x01);
            Spi_SendData(0x03);
            Spi_SendData(0x07);
            Spi_SendData(0x0f);
            Spi_SendData(0x1f);
            Spi_SendData(0x3f);
            Spi_SendData(0x7f);
            Spi_SendData(0xff);

            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
            Spi_SendData(0xff);
    }
            // Spi_SendData(0x01);
            // Spi_SendData(0x03);
            // Spi_SendData(0x07);
            // Spi_SendData(0x0f);
            // Spi_SendData(0x1f);
            // Spi_SendData(0x3f);
            // Spi_SendData(0x7f);
            // Spi_SendData(0xff);

            // Spi_SendData(0x01);
            // Spi_SendData(0x03);
            // Spi_SendData(0x07);
            // Spi_SendData(0x0f);
            // Spi_SendData(0x1f);
            // Spi_SendData(0x3f);
            // Spi_SendData(0x7f);
            // Spi_SendData(0xff);

    // for (; i < 152; i++)
    // {
    //     for (j = 0; j < 19; j++)
    //     {
    //         Spi_SendData(data);
    //     }
    // }


    // for (i = 0; i < 52; i++)
    // {
    //     for (j = 0; j < 9; j++)
    //     {
    //         Spi_SendData(data);
    //     }

    //     for (j = 10; j < 19; j++)
    //     {
    //         Spi_SendData(0xff);
    //     }

    // }
    // for (i = 53; i < 152; i++)
    // {
    //     for (j = 0; j < 19; j++)
    //     {
    //         Spi_SendData(0xff);
    //     }
    // }

    Spi_SetCS(TRUE);
    DC_L;
    delay1ms(2);

    spiWriteCmd(0x12);
} 

void EPD_poweroff(void)
{
    spiWriteCmd(0x02);
    spiWriteCmd(0x07);
    spiWriteData(0xA5); 
}

void EPD_display(unsigned char data[][BYTES_PER_ROW])
{
    int i = 0,j = 0;
    spiWriteCmd(0x13);
    delay1ms(2);

    DC_H;
    Spi_SetCS(TRUE);
    Spi_SetCS(FALSE);
    for (i = 0; i < 152; i++)
    {
        for (j = 0; j < 19; j++)
        {
            Spi_SendData(data[i][j]);
        }
    }

    Spi_SetCS(TRUE);
    DC_L;
    delay1ms(2);

    spiWriteCmd(0x12);
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


