//------------Copyright (C) 2012 Maxim Integrated Products --------------
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL MAXIM INTEGRATED PRODCUTS BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Maxim Integrated Products
// shall not be used except as stated in the Maxim Integrated Products
// Branding Policy.
// ---------------------------------------------------------------------------
//
// 1wire_ds2465.h - Include file for DS2465
//

// API mode bit flags
#define MODE_NORMAL                    0x00
#define MODE_STANDARD                  0x00
#define MODE_OVERDRIVE                 0x01
#define MODE_STRONG                    0x02

// DS2465 commands
#define CMD_1WMR   0xF0
#define CMD_WCFG   0xD2
#define CMD_CHSL   0xC3
#define CMD_SRP    0xE1

#define CMD_1WRS   0xB4
#define CMD_1WWB   0xA5
#define CMD_1WRB   0x96
#define CMD_1WSB   0x87
#define CMD_1WT    0x78
#define CMD_1WTB   0x69
#define CMD_1WRF   0xE1
#define CMD_CPS    0x5A
#define CMD_CSS    0x4B
#define CMD_CSAM   0x3C
#define CMD_CSWM   0x2D
#define CMD_CNMS   0x1E
#define CMD_SPR    0x0F

// DS2465 config bits
#define CONFIG_APU  0x01
#define CONFIG_PDN  0x02
#define CONFIG_SPU  0x04
#define CONFIG_1WS  0x08

// DS2465 status bits
#define STATUS_1WB  0x01
#define STATUS_PPD  0x02
#define STATUS_SD   0x04
#define STATUS_LL   0x08
#define STATUS_RST  0x10
#define STATUS_SBR  0x20
#define STATUS_TSB  0x40
#define STATUS_DIR  0x80

// addresses
#define ADDR_SPAD       0x00
#define ADDR_CMD_REG    0x60
#define ADDR_STATUS_REG 0x61
#define ADDR_DATA_REG   0x62
#define ADDR_MAC_READ   0x63
#define ADDR_SHA_SELECT_REG 0x66
#define ADDR_WCFG_REG   0x67

#define ADDR_TRSTL_REG  0x68
#define ADDR_TMSP_REG   0x69
#define ADDR_TW0L_REG   0x6A
#define ADDR_TREC0_REG  0x6B
#define ADDR_RWPU_REG   0x6C
#define ADDR_TW1L_REG   0x6D

// region
#define REGION_FULL_PAGE   0x03
#define REGION_FIRST_HALF  0x01
#define REGION_SECOND_HALF 0x02

// I2C address for DS2465
#define I2C_ADDR_DS2465           0x30

// delays (if not polling for complet)
#define DS2465_EEPROM_WRITE_DELAY    30
#define DS2465_LOAD_SECRET_DELAY     90
#define DS2465_SHA_COMPUTATION_DELAY 5



#ifndef uchar
   typedef unsigned char uchar;
#endif


#ifndef DS2465
   // 1-Wire API for DSKCP1 function prototypes
   extern int OWReset(void);
   extern void OWWriteByte(uchar sendbyte);
   extern uchar OWReadByte(void);
   extern uchar OWTouchByte(uchar sendbyte);
   extern uchar OWTouchBit(uchar sendbit);
   extern void OWWriteBit(uchar sendbit);
   extern int OWReadBit(void);
   extern void OWBlock(uchar *tran_buf, int tran_len);
   extern int OWFirst(void);
   extern int OWNext(void);
   extern int  OWVerify(void);
   extern void OWTargetSetup(uchar family_code);
   extern void OWFamilySkipSetup(void);
   extern int OWSearch(void);

   extern int OWReadROM(void);
   extern int OWSkipROM(void);
   extern int OWMatchROM(void);
   extern int OWOverdriveSkipROM(void);
   extern int OWResume(void);
   extern int OWOverdriveMatchROM(void);

   // Extended 1-Wire functions
   extern int OWSpeed(int new_speed);
   extern int OWLevel(int level);
   extern void OWWriteBytePower(uchar sendbyte);
   extern uchar OWReadBytePower(void);
   extern int OWReadBitPower(int applyPowerResponse);
   extern int OWTransmitBlock(int tx_mac, uchar *tran_buf, int tran_len);
   extern int OWReceiveBlock(uchar *rx_buf, int rx_len);
   extern int OWPowerDown(void);
   extern int OWPowerUp(void);

   extern int DS2465_OWConfigureRWPU(int readflag, int value);
   extern int DS2465_OWConfigureTREC0(int readflag, int ovr, int std);
   extern int DS2465_OWConfigureTW0L(int readflag, int ovr, int std);
   extern int DS2465_OWConfigureTMSP(int readflag, int ovr, int std);
   extern int DS2465_OWConfigureTRSTL(int readflag, int ovr, int std);
   extern int DS2465_OWConfigureTW1L_Overdrive(int readflag, int ovr);
   extern int DS2465_OWConfigureAPU(int apu_enable);

   // Helper functions
   extern int DS2465_detect();
   extern uchar DS2465_search_triplet(int search_direction);
   extern int DS2465_write_config(uchar config);
   extern int DS2465_reset(void);
   extern uchar calc_crc8(uchar data);
   extern int DS2465_read(int addr, uchar *buf, int len, int skip_set_pointer);

   // last device number found
   extern uchar ROM_NO[8];

#endif
