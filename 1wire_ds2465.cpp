//------------Copyright (C) 2013 Maxim Integrated Products --------------
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
//  1wire_ds2465.c - DS2465 1-Wire example module.
//      Requires low level I2C connection.
//

#include <stdio.h>
#include <string.h>


#define DS2465
#include "1wire_ds2465.h"

#include "gpiotools.h"
#define I2C_ON_IMX6S

// 1-Wire API for DS2465 function prototypes
int OWReset(void);
void OWWriteByte(uchar sendbyte);
uchar OWReadByte(void);
uchar OWTouchByte(uchar sendbyte);
uchar OWTouchBit(uchar sendbit);
void OWWriteBit(uchar sendbit);
int OWReadBit(void);
void OWBlock(uchar *tran_buf, int tran_len);
int OWFirst(void);
int OWNext(void);
int  OWVerify(void);
void OWTargetSetup(uchar family_code);
void OWFamilySkipSetup(void);
int OWSearch(void);
int OWReadROM(void);
int OWSkipROM(void);
int OWMatchROM(void);
int OWOverdriveSkipROM(void);
int OWResume(void);

// Extended 1-Wire functions
int OWSpeed(int new_speed);
int OWLevel(int level);
void OWWriteBytePower(uchar sendbyte);
uchar OWReadBytePower(void);
int OWReadBitPower(int applyPowerResponse);
int OWTransmitBlock(int tx_mac, uchar *tran_buf, int tran_len);
int OWReceiveBlock(uchar *rx_buf, int rx_len);
int OWPowerDown(void);
int OWPowerUp(void);
int DS2465_OWConfigureRWPU(int readflag, int value);
int DS2465_OWConfigureTREC0(int readflag, int ovr, int std);
int DS2465_OWConfigureTW0L(int readflag, int ovr, int std);
int DS2465_OWConfigureTMSP(int readflag, int ovr, int std);
int DS2465_OWConfigureTRSTL(int readflag, int ovr, int std);
int DS2465_OWConfigureTW1L_Overdrive(int readflag, int ovr);
int DS2465_OWConfigureAPU(int apu_enable);

// Memory operations
int DS2465_write_SRAM(int addr, uchar *buf, int len);
int DS2465_read(int addr, uchar *buf, int len, int skip_set_pointer);

// Helper functions
int DS2465_detect();
uchar DS2465_search_triplet(int search_direction);
int DS2465_write_config(uchar config);
int DS2465_reset(void);
uchar calc_crc8(uchar data);
int DS2465_Command(uchar cmd, uchar par, int poll);
int DS2465_ow_config(int readflag, int ovr, int std, int addr);

// Search state
uchar ROM_NO[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
uchar crc8;

// DS2465 state
uchar I2C_address;
int short_detected;
int c1WS, cSPU, cPDN, cAPU;

// Functions to display debug
int owdebug = false;
extern int pause_before_spbyte;

/*
// extenral debug and delay functions
extern int dprintf(char *format, ...);
extern void msDelay(int delay);

*/
// flag to enable or disable block usage
int USE_BLOCKS=0;

//---------------------------------------------------------------------------
//-------- DS2465 Helper functions
//---------------------------------------------------------------------------

//--------------------------------------------------------------------------
// DS2428 Detect routine that sets the I2C address and then performs a
// device reset followed by writing the configuration byte to default values:
//   1-Wire speed (c1WS) = standard (0)
//   Strong pull-up (cSPU) = off (0)
//   Presence pulse masking (cPDN) = off (0)
//   Active pull-up (cAPU) = on (CONFIG_APU = 0x01)
//
// Returns: TRUE if device was detected and written
//          FALSE device not detected or failure to write configuration byte
//
int DS2465_detect()
{
   // reset the DS2465 ON selected address
   if (!DS2465_reset())
      return false;

   // default configuration
   c1WS = false;
   cSPU = false;
   cPDN = false;
   cAPU = CONFIG_APU;

   // write the default configuration setup
   if (!DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU)))
      return false;

   return true;
}

//--------------------------------------------------------------------------
// Perform a device reset on the DS2465
//
// Returns: TRUE if device was reset
//          FALSE device not detected or failure to perform reset
//
int DS2465_reset(void)
{

   uchar status;
    uchar write_block[2];
   int result;

   // Device Reset
   //   S AD,0 [A] ADDR_CMD_REG [A] 1WMR [A] Sr AD,1 [A] [SS] A\ P
   //  [] indicates from slave
   //  SS status byte to read to verify state
#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_CMD_REG;
    write_block[cnt++] = CMD_1WMR;
    //if(!gpioTools::I2C_Write(write_block, cnt, EXPECT_ACK))
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }
    status = gpioTools::I2C_read(NACK,&result);
    gpioTools::I2C_stop();

#else
   if (USE_BLOCKS)
   {
      write_block[0] = CMD_1WMR;

      if (!I2C_writeReadBlock(I2C_address, ADDR_CMD_REG, 1, &write_block[0], 1, &status))
         return FALSE;
   }
   else
   {

      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(CMD_1WMR, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      status = I2C_read(NACK,&result);
      if (!result)
      {
         I2C_stop();
         return FALSE;
      }

      I2C_stop();
   }
#endif
   // do a command to get 1-Wire master reset out of holding state
   OWReset();

   // check for failure due to incorrect read back of status
   return ((status & 0xF7) == 0x10);
}

//--------------------------------------------------------------------------
// Write the configuration register in the DS2465. The configuration
// options are provided in the lower nibble of the provided config byte.
// The uppper nibble in bitwise inverted when written to the DS2465.
//
// Returns:  TRUE: config written and response correct
//           FALSE: response incorrect
//
int DS2465_write_config(uchar config)
{
   uchar read_config;
   uchar buf[1], write_block[2];
   int result;

#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_WCFG_REG;
    write_block[cnt++] = (uchar)(config | (~config << 4));
    //if(!I2C_Write(write_block, cnt, EXPECT_ACK))
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }
    gpioTools::I2C_stop();

    // read it back to confirm
    // S AD,0 [A] ADDR_WCFG_REG [A] Sr AD,1 [A] [CF] A\

    cnt = 0;
    gpioTools::I2C_start();
    write_block[cnt++] = ADDR_WCFG_REG;
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }
    gpioTools::I2C_rep_start();

    read_config = gpioTools::I2C_read(NACK,&result);
    if (!result)
    {
       gpioTools::I2C_stop();
       return false;
    }

    gpioTools::I2C_stop();

#else

   if (USE_BLOCKS)
   {
      buf[0] = (config | (~config << 4));

      // write and then read to confirm
      if (!I2C_writeBlock(I2C_address, ADDR_WCFG_REG, 1, buf))
         return FALSE;

      if (!I2C_readBlock(I2C_address, ADDR_WCFG_REG, 1, &read_config, FALSE))
         return FALSE;
   }
   else
   {
      // Write configuration byte
      //   S AD,0 [A] ADDR_WCFG_REG [A] CONIG [A] P
      //  [] indicates from slave
      //  CF configuration byte to write

      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(ADDR_WCFG_REG, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write((uchar)(config | (~config << 4)), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      I2C_stop();

      // read it back to confirm
      // S AD,0 [A] ADDR_WCFG_REG [A] Sr AD,1 [A] [CF] A\

      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(ADDR_WCFG_REG, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      read_config = I2C_read(NACK,&result);
      if (!result)
      {
         I2C_stop();
         return FALSE;
      }

      I2C_stop();
   }
#endif
   // check for failure due to incorrect read back
   if (config != read_config)
   {
      // handle error
      // ...
      DS2465_reset();

      return false;
   }


   return true;
}

//--------------------------------------------------------------------------
// Write to SRAM memory on the DS2465
//
// 'addr' - address to start writing (must be in SRAM)
// 'buf' - buffer of data to write
// 'len' - length to write
//
// Returns: TRUE write successful
//          FALSE failure to complete write
//
int DS2465_write_SRAM(int addr, uchar *buf, int len)
{
   int i;

   // Write SRAM (Case A)
   //   S AD,0 [A] VSA [A] DD [A]  P
   //                      \-----/
   //                        Repeat for each data byte
   //  [] indicates from slave
   //  VSA valid SRAM memory address
   //  DD memory data to write
#ifdef I2C_ON_IMX6S
    int cnt=0;
    uchar write_block[50];

    gpioTools::I2C_start();

    write_block[cnt++] = addr;
    for(i=0; i<len; i++)
    {
        write_block[cnt++] = buf[i];
    }

    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }
    gpioTools::I2C_stop();

#else


   if (USE_BLOCKS)
   {
      return I2C_writeBlock(I2C_address, addr, len, buf);
   }
   else
   {
      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write((uchar)addr, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      // loop to write each byte
      for (i = 0; i < len; i++)
      {
         if (!I2C_write(buf[i], EXPECT_ACK))
         {
            I2C_stop();
            return FALSE;
         }
      }
      I2C_stop();
   }
#endif
   return true;
}


//--------------------------------------------------------------------------
// Read memory from the DS2465
//
// 'addr' - address to start reading
// 'buf' - buffer to hold memory read
// 'len' - length to read
// 'skip_set_pointer' - flag to indicate to skip setting address pointer
//
// Returns: TRUE read successful
//          FALSE failure to complete read
//
int DS2465_read(int addr, uchar *buf, int len, int skip_set_pointer)
{
   int ack, i, result;

   // Read (Case A)
   //   S AD,0 [A] MA [A] Sr AD,1 [A] [DD] A [DD] A\ P
   //                                 \-----/
   //                                   Repeat for each data byte, NAK last byte
   //  [] indicates from slave
   //  MA memory address
   //  DD memory data read

#ifdef I2C_ON_IMX6S
    int cnt=0;
    uchar write_block[2];

    gpioTools::I2C_start();

    if(!skip_set_pointer)
    {
        write_block[cnt++] = addr;
        if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
        {
            gpioTools::I2C_stop();
            return false;
        }
    }

    for (i = 0; i < len; i++)
    {
       if (i == (len -1))
          ack = NACK;
       else
          ack = ACK;
       buf[i] = gpioTools::I2C_read(ack,&result);
       if (!result)
       {
          gpioTools::I2C_stop();
          return false;
       }
    }
    gpioTools::I2C_stop();


#else


   if (USE_BLOCKS)
   {
      return I2C_readBlock(I2C_address, addr, len, buf, skip_set_pointer);
   }
   else
   {
      I2C_start();
      if (!skip_set_pointer)
      {
         if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
         {
            I2C_stop();
            return FALSE;
         }
         if (!I2C_write((uchar)addr, EXPECT_ACK))
         {
            I2C_stop();
            return FALSE;
         }
         I2C_rep_start();
      }

      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      // loop to read each byte, NAK last byte
      for (i = 0; i < len; i++)
      {
         if (i == (len -1))
            ack = NACK;
         else
            ack = ACK;
         buf[i] = I2C_read(ack,&result);
         if (!result)
         {
            I2C_stop();
            return FALSE;
         }
      }
      I2C_stop();
   }
#endif

   return true;
}



//--------------------------------------------------------------------------
// Copy Scratchpad on DS2465 to either secret or memory page
//
// 'dest_secret' - 1 if destination is secret, 0 if memory page
// 'page' - page number if dest_secret=0
// 'notfull' - 0 if only 4 byte segment, 1 if writing to full page,
// 'seg' - Segment number if full=0.
//
// Returns: TRUE write successful
//          FALSE failure to complete read
//
int DS2465_CopyScratchpad(int dest_secret, int page, int notfull, int seg)
{
   uchar par;

   // create parameter byte
   if (dest_secret)
      par = 0;
   else
      par = (uchar)(0x80 | (page << 4) | (notfull << 3) | seg);

   return DS2465_Command(CMD_CPS, par, true);
}

//--------------------------------------------------------------------------
// Compute S-Secret on DS2465
//
// 'swap' - 1 if swapping a page into the computation
// 'page' - page number to swap in
// 'region' - (1) first 1/2 page, (2) second 1/2 page, (3) entire page
//
// Returns: TRUE write successful
//          FALSE failure to complete read
//
int DS2465_Compute_SSecret(int swap, int page, int region)
{
   uchar par;

   // create parameter byte
   if (!swap)
      par = 0xBF;
   else
      par = (uchar)(0xC8 | (page << 4) | region);

   return DS2465_Command(CMD_CSS, par, true);
}

//--------------------------------------------------------------------------
// Compute Slave Authentication MAC DS2465
//
// 'swap' - 1 if swapping a page into the computation
// 'page' - page number to swap in
// 'region' - (1) first 1/2 page, (2) second 1/2 page, (3) entire page
//
// Returns: TRUE write successful
//          FALSE failure to complete read
//
int DS2465_Compute_AuthMAC(int swap, int page, int region)
{
   uchar par;

   // create parameter byte
   if (!swap)
      par = 0xBF;
   else
      par = (uchar)(0xC8 | (page << 4) | region);

   return DS2465_Command(CMD_CSAM, par, false);
}

//--------------------------------------------------------------------------
// Compute Write MAC DS2465
//
// 'regwrite' - 1 if writing to a register, 0 if regular memory
// 'swap' - 1 if swapping a page into the computation
// 'page' - page number to swap in
// 'segment' - segment number if swaping
//
// Returns: TRUE write successful
//          FALSE failure to complete read
//
int DS2465_Compute_WriteMAC(int regwrite, int swap, int page, int segment)
{
   uchar par;

   // create parameter byte
   par = (uchar)((regwrite << 7) | (swap << 6) | (page << 4) | segment);

   return DS2465_Command(CMD_CSWM, par, false);
}

//--------------------------------------------------------------------------
// Compute Next Master Secret DS2465
//
// 'regwrite' - 1 if writing to a register, 0 if regular memory
// 'swap' - 1 if swapping a page into the computation
// 'page' - page number to swap in
// 'region' - (1) first 1/2 page, (2) second 1/2 page, (3) entire page
//
// Returns: TRUE write successful
//          FALSE failure to complete read
//
int DS2465_Compute_NextMasterSecret(int swap, int page, int region)
{
   uchar par;

   // create parameter byte
   if (!swap)
      par = 0xBF;
   else
      par = (uchar)(0xC8 | (page << 4) | region);

   return DS2465_Command(CMD_CNMS, par, true);
}


//--------------------------------------------------------------------------
// Copy Scratchpad on DS2465 to either secret or memory page
//
// 'dest_secret' - 1 if destination is secret, 0 if memory page
// 'page' - page number if dest_secret=0
// 'full' - 1 if writing to full page, 0 if only 4 byte segment
// 'seg' - Segment number if full=0.
//
// Returns: TRUE write successful
//          FALSE failure to complete read
//
int DS2465_Command(uchar cmd, uchar par, int poll)
{
   int poll_count=0,status;
   uchar buf[2];

   // Generic command
   //   S AD,0 [A] ADDR_CMD_REG [A] CMD [A] PP [A]  P
   //  [] indicates from slave
   //  CMD command
   //  PP parameter

   if (USE_BLOCKS)
   {
      buf[0] = cmd;
      buf[1] = par;

      if (!gpioTools::I2C_writeBlock(I2C_address, ADDR_CMD_REG, 2, buf))
         return false;
   }
   else
   {
      gpioTools::I2C_start();
      if (!gpioTools::I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return false;
      }
      if (!gpioTools::I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return false;
      }
      if (!gpioTools::I2C_write(cmd, EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return false;
      }
      if (!gpioTools::I2C_write(par, EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return false;
      }

      gpioTools::I2C_stop();
   }

   poll_count = 0;
   if (poll)
   {
      // Poll for completion by checking for NAK on address
      status = 0;
      while (!status && (poll_count++ < POLL_LIMIT))
      {
         gpioTools::I2C_start();
         status = gpioTools::I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK);
         gpioTools::I2C_stop();
      }
   }
   else
   {
      // delay instead of poll, longest operation (only for SHA compute)
      gpioTools::msDelay(DS2465_SHA_COMPUTATION_DELAY * 2);
      gpioTools::msDelay(8); // Additional delay
   }

   // check for failure due to poll limit reached
   if (poll_count >= POLL_LIMIT)
   {
      // handle error
      // ...
      DS2465_reset();
      return false;
   }

   return true;
}

//---------------------------------------------------------------------------
//-------- Basic 1-Wire functions
//---------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Reset all of the devices on the 1-Wire Net and return the result.
//
// Returns: TRUE(1):  presense pulse(s) detected, device(s) reset
//          FALSE(0): no presense pulses detected
//
int OWReset(void)
{
   uchar status;
   uchar write_block[2];
   int poll_count = 0, result;

   // debug
   if (owdebug)
   {
      if (c1WS)
         ;//printf("\n<OVR> R");
      else
         ;//printf("\n<STD> R");
   }

   // 1-Wire reset (Case B)
   //   S AD,0 [A] ADDR_CMD_REG  [A] 1WRS [A] Sr AD,1 [A] [Status] A [Status] A\ P
   //                                                  \--------/
   //                       Repeat until 1WB bit has changed to 0
   //  [] indicates from slave
#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_CMD_REG;
    write_block[cnt++] = CMD_1WRS;
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }

    status = STATUS_1WB;
    while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
    {
       status = gpioTools::I2C_read(ACK,&result);
       if (!result)
       {
           gpioTools::I2C_stop();
          return false;
       }
    }

    // one last read with NACK
    gpioTools::I2C_read(NACK,&result);
    if (!result)
    {
        gpioTools::I2C_stop();
       return false;
    }

    gpioTools::I2C_stop();

    // check for failure due to poll limit reached
    if (poll_count >= POLL_LIMIT)
    {
       // handle error
       // ...
       DS2465_reset();
       return false;
    }

#else
   if (USE_BLOCKS)
   {
      write_block[0] = ADDR_CMD_REG;
      write_block[1] = CMD_1WRS;

      if (!I2C_readBlockPoll(I2C_address, 2, &write_block[0], 0xFF, STATUS_1WB, &status))
         return FALSE;
   }
   else
   {
      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(CMD_1WRS, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }

      // loop checking 1WB bit for completion of 1-Wire operation
      // abort if poll limit reached
      status = STATUS_1WB;
      while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
      {
         status = I2C_read(ACK,&result);
         if (!result)
         {
            I2C_stop();
            return FALSE;
         }
      }

      // one last read with NACK
      I2C_read(NACK,&result);
      if (!result)
      {
         I2C_stop();
         return FALSE;
      }

      I2C_stop();

      // check for failure due to poll limit reached
      if (poll_count >= POLL_LIMIT)
      {
         // handle error
         // ...
         DS2465_reset();
         return FALSE;
      }
   }
#endif
   // check for short condition
   if (status & STATUS_SD)
      short_detected = true;
   else
      short_detected = false;

   // debug
   if (owdebug)
   {
      if (status & STATUS_PPD)
         ;//printf("P ");
      else
         ;//printf("N ");
   }

   // check for presence detect
   if (status & STATUS_PPD)
      return true;
   else
      return false;
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net.
// The parameter 'sendbit' least significant bit is used.
//
// 'sendbit' - 1 bit to send (least significant byte)
//
void OWWriteBit(uchar sendbit)
{
   // debug
   if (owdebug)
      ;//printf("%d ",sendbit);

   OWTouchBit(sendbit);
}

//--------------------------------------------------------------------------
// Reads 1 bit of communication from the 1-Wire Net and returns the
// result
//
// Returns:  1 bit read from 1-Wire Net
//
int OWReadBit(void)
{
   unsigned char rt;

   rt = OWTouchBit(0x01);

   // debug
   if (owdebug)
      ;//printf("[%d] ",rt);

   return rt;
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and return the
// result 1 bit read from the 1-Wire Net.  The parameter 'sendbit'
// least significant bit is used and the least significant bit
// of the result is the return bit.
//
// 'sendbit' - the least significant bit is the bit to send
//
// Returns: 0:   0 bit read from sendbit
//          1:   1 bit read from sendbit
//
uchar OWTouchBit(uchar sendbit)
{
   uchar status;
   uchar write_block[3];
   int poll_count = 0, result;

   // 1-Wire bit (Case B)
   //   S AD,0 [A] ADDR_CMD_REG [A] 1WSB [A] BB [A] Sr AD,1 [A] [Status] A [Status] A\ P
   //                                                          \--------/
   //                           Repeat until 1WB bit has changed to 0
   //  [] indicates from slave
   //  BB indicates byte containing bit value in msbit

   if (USE_BLOCKS)
   {
      write_block[0] = ADDR_CMD_REG;
      write_block[1] = CMD_1WSB;
      write_block[2] = (sendbit ? 0x80 : 0x00);

      gpioTools::I2C_readBlockPoll(I2C_address, 3, &write_block[0], 0xFF, STATUS_1WB, &status);
   }
   else
   {
      gpioTools::I2C_start();
      if (!gpioTools::I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return 0;
      }
      if (!gpioTools::I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return 0;
      }
      if (!gpioTools::I2C_write(CMD_1WSB, EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return 0;
      }
      if (!gpioTools::I2C_write((uchar)(sendbit ? 0x80 : 0x00), EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return 0;
      }
      gpioTools::I2C_rep_start();
      if (!gpioTools::I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         gpioTools::I2C_stop();
         return 0;
      }

      // loop checking 1WB bit for completion of 1-Wire operation
      // abort if poll limit reached
      status = STATUS_1WB;
      while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
      {
         status = gpioTools::I2C_read(status & STATUS_1WB,&result);
         if (!result)
         {
            gpioTools::I2C_stop();
            return false;
         }
      }

      // one last read with NACK
      gpioTools::I2C_read(NACK,&result);
      if (!result)
      {
         gpioTools::I2C_stop();
         return false;
      }

      gpioTools::I2C_stop();

      // check for failure due to poll limit reached
      if (poll_count >= POLL_LIMIT)
      {
         // handle error
         // ...
         DS2465_reset();
         return 0;
      }
   }

   // debug
   if (owdebug)
   {
      if (status & STATUS_SBR)
         ;//printf("1 ");
      else
         ;//printf("0 ");
   }

   // return bit state
   if (status & STATUS_SBR)
      return 1;
   else
      return 0;
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net is the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  TRUE: bytes written and echo was the same
//           FALSE: echo was not the same
//
void OWWriteByte(uchar sendbyte)
{
   uchar status;
   uchar write_block[3];
   int poll_count = 0, result;

   // debug
   if (owdebug)
      ;//printf("%02X ",sendbyte);

   // 1-Wire Write Byte (Case B)
   //   S AD,0 [A] ADDR_CMD_REG [A] 1WWB [A] DD [A] Sr AD,1 [A] [Status] A [Status] A\ P
   //                                                           \--------/
   //                             Repeat until 1WB bit has changed to 0
   //  [] indicates from slave
   //  DD data to write

#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_CMD_REG;
    write_block[cnt++] = CMD_1WWB;
    write_block[cnt++] = sendbyte;

    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return ;
    }

    gpioTools::I2C_rep_start();

    // loop checking 1WB bit for completion of 1-Wire operation
    // abort if poll limit reached
    status = STATUS_1WB;
    while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
    {
       status = gpioTools::I2C_read(ACK,&result);
       if (!result)
       {
          gpioTools::I2C_stop();
          return;
       }
    }

    // one last read with NACK
    gpioTools::I2C_read(NACK,&result);
    if (!result)
    {
       gpioTools::I2C_stop();
       return;
    }

    gpioTools::I2C_stop();

    // check for failure due to poll limit reached
    if (poll_count >= POLL_LIMIT)
    {
       // handle error
       // ...
       DS2465_reset();
    }


#else



   if (USE_BLOCKS)
   {
      write_block[0] = ADDR_CMD_REG;
      write_block[1] = CMD_1WWB;
      write_block[2] = sendbyte;

      I2C_readBlockPoll(I2C_address, 3, &write_block[0], 0xFF, STATUS_1WB, &status);
   }
   else
   {
      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return;
      }
      if (!I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         I2C_stop();
         return;
      }
      if (!I2C_write(CMD_1WWB, EXPECT_ACK))
      {
         I2C_stop();
         return;
      }
      if (!I2C_write(sendbyte, EXPECT_ACK))
      {
         I2C_stop();
         return;
      }

      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         I2C_stop();
         return;
      }

      // loop checking 1WB bit for completion of 1-Wire operation
      // abort if poll limit reached
      status = STATUS_1WB;
      while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
      {
         status = I2C_read(ACK,&result);
         if (!result)
         {
            I2C_stop();
            return;
         }
      }

      // one last read with NACK
      I2C_read(NACK,&result);
      if (!result)
      {
         I2C_stop();
         return;
      }

      I2C_stop();

      // check for failure due to poll limit reached
      if (poll_count >= POLL_LIMIT)
      {
         // handle error
         // ...
         DS2465_reset();
      }
   }
#endif
}

//--------------------------------------------------------------------------
// Send 8 bits of read communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.
//
// Returns:  8 bits read from 1-Wire Net
//
uchar OWReadByte(void)
{
   uchar data, status;
   uchar write_block[2];
   int poll_count = 0, result;

   // 1-Wire Read Bytes (Case C)
   //   S AD,0 [A] ADDR_CMD_REG [A] 1WRB [A] Sr AD,1 [A] [Status] A [Status] A\
   //                                                  \--------/
   //                     Repeat until 1WB bit has changed to 0
   //   Sr AD,0 [A] SRP [A] E1 [A] Sr AD,1 [A] DD A\ P
   //
   //  [] indicates from slave
   //  DD data read

#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_CMD_REG;
    write_block[cnt++] = CMD_1WRB;
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }

    gpioTools::I2C_rep_start();

    // loop checking 1WB bit for completion of 1-Wire operation
    // abort if poll limit reached
    status = STATUS_1WB;
    while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
    {
       status = gpioTools::I2C_read(ACK,&result);
       if (!result)
       {
          gpioTools::I2C_stop();
          return false;
       }
    }

    // one last read with NACK
    gpioTools::I2C_read(NACK,&result);
    if (!result)
    {
       gpioTools::I2C_stop();
       return false;
    }


    // check for failure due to poll limit reached
    if (poll_count >= POLL_LIMIT)
    {
       // handle error
       // ...
       DS2465_reset();
       return 0;
    }

    gpioTools::I2C_rep_start();

    cnt = 0;

    write_block[cnt++] = ADDR_DATA_REG;
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }

    gpioTools::I2C_rep_start();

    data =  gpioTools::I2C_read(NACK,&result);
    if (!result)
    {
       gpioTools::I2C_stop();
       return false;
    }

    gpioTools::I2C_stop();

#else


   if (USE_BLOCKS)
   {
      write_block[0] = ADDR_CMD_REG;
      write_block[1] = CMD_1WRB;

      // do read byte and wait for complete
      if (!I2C_readBlockPoll(I2C_address, 2, &write_block[0], 0xFF, STATUS_1WB, &status))
         return FALSE;

      // read the data
      if (!I2C_readBlock(I2C_address, ADDR_DATA_REG, 1, &data, FALSE))
         return FALSE;
   }
   else
   {
      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      if (!I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      if (!I2C_write(CMD_1WRB, EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
         return 0;

      // loop checking 1WB bit for completion of 1-Wire operation
      // abort if poll limit reached
      status = STATUS_1WB;
      while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
      {
         status = I2C_read(ACK,&result);
         if (!result)
         {
            I2C_stop();
            return FALSE;
         }
      }

      // one last read with NACK
      I2C_read(NACK,&result);
      if (!result)
      {
         I2C_stop();
         return FALSE;
      }


      // check for failure due to poll limit reached
      if (poll_count >= POLL_LIMIT)
      {
         // handle error
         // ...
         DS2465_reset();
         return 0;
      }

      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      if (!I2C_write(ADDR_DATA_REG, EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      data =  I2C_read(NACK,&result);
      if (!result)
      {
         I2C_stop();
         return FALSE;
      }

      I2C_stop();
   }
#endif
   // debug
   if (owdebug)
   {
      ;//printf("[%02X] ",data);
   }

   return data;
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.  The parameter 'sendbyte'
// least significant 8 bits are used and the least significant 8 bits
// of the result is the return byte.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  8 bits read from sendbyte
//
uchar OWTouchByte(uchar sendbyte)
{
   if (sendbyte == 0xFF)
      return OWReadByte();
   else
   {
      OWWriteByte(sendbyte);
      return sendbyte;
   }
}

//--------------------------------------------------------------------------
// The 'OWBlock' transfers a block of data to and from the
// 1-Wire Net. The result is returned in the same buffer.
//
// 'tran_buf' - pointer to a block of unsigned
//              chars of length 'tran_len' that will be sent
//              to the 1-Wire Net
// 'tran_len' - length in bytes to transfer
//
void OWBlock(uchar *tran_buf, int tran_len)
{
   int i;

   for (i = 0; i < tran_len; i++)
      tran_buf[i] = OWTouchByte(tran_buf[i]);
}

//--------------------------------------------------------------------------
// The 'OWTransmitBlock' transfers a block of data to the
// 1-Wire Net. The mac buffer can be sent (tx_mac=1) or a
// portion of the scratchpad can be sent.
//
// 'tx_mac'   - flag to indicate if the MAC buffer is to be sent (1) or
//              the data provided in teh tran_buf is to be sent (0)
// 'tran_buf' - pointer to a block of bytes
//              of length 'tran_len' that will be sent
//              to the 1-Wire Net
// 'tran_len' - length in bytes to transfer. Only valid numbers are 8,16,20,32;
//
int OWTransmitBlock(int tx_mac, uchar *tran_buf, int tran_len)
{
   uchar par;
   uchar status[1];
   uchar write_block[3];
   int poll_count=0,i;

   // debug output
   if (owdebug)
   {
      for (i = 0; i < tran_len; i++)
         ;//printf("%02X ",tran_buf[i]);
   }

   // create parameter byte
   if (tx_mac)
      par = 0xFF;
   else
   {
      // scratchpad is source
      par = tran_len;

      // prefill scratchpad with required data
      if (!DS2465_write_SRAM(ADDR_SPAD, tran_buf, tran_len))
         return false;
   }

   // 1-Wire Transmit Block (Case A)
   //   S AD,0 [A] ADDR_CMD_REG [A] 1WTB [A] PR [A] P
   //  [] indicates from slave
   //  PR indicates byte containing parameter

#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_CMD_REG;
    write_block[cnt++] = CMD_1WTB;
    write_block[cnt++] = par;
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }

    gpioTools::I2C_stop();

#else

   if (USE_BLOCKS)
   {
      write_block[0] = CMD_1WTB;
      write_block[1] = par;

      I2C_writeBlock(I2C_address, ADDR_CMD_REG, 2, write_block);

      // Use I2C_readBlockPoll to wait for result
      write_block[0] = ADDR_STATUS_REG;
      if (I2C_readBlockPoll(I2C_address, 1, write_block, 0xFF, STATUS_1WB, &status[0]))
      {
         // check status to see if complete
         if (!(status[0] & STATUS_1WB))
            return TRUE;
      }
      // fall through to manual polling
   }
   else
   {
      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(CMD_1WTB, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(par, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      I2C_stop();
   }
#endif
   // loop checking 1WB bit for completion of 1-Wire operation
   // abort if poll limit reached
   status[0] = STATUS_1WB;
   while ((status[0] & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
   {
      if (!DS2465_read(ADDR_STATUS_REG,status,1,false))
         return false;
   }

   // check for failure due to poll limit reached
   if (poll_count >= POLL_LIMIT)
   {
      // handle error
      // ...
      DS2465_reset();
      return false;
   }

   return true;
}

//--------------------------------------------------------------------------
// The 'OWReceiveBlock' receives a block of data from the
// 1-Wire Net. The destination is the mac buffer (rx_mac=1) or
// the scratchpad (rx_mac=0). The result is buffer is returned.
//
// 'rx_buf'   - pointer to a block to receive bytes
//              of length 'rx_len' from 1-Wire Net
// 'rx_len' - length in bytes to read. Only valid numbers are 8,16,20,32;
//
int OWReceiveBlock(uchar *rx_buf, int rx_len)
{
   uchar status[1];
   uchar write_block[3];
   int poll_count=0,i,rt;

   // 1-Wire Receive Block (Case A)
   //   S AD,0 [A] ADDR_CMD_REG [A] 1WRF [A] PR [A] P
   //  [] indicates from slave
   //  PR indicates byte containing parameter

#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_CMD_REG;
    write_block[cnt++] = CMD_1WRF;
    write_block[cnt++] = (uchar)rx_len;
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }
    gpioTools::I2C_stop();
    status[0] = STATUS_1WB;

#else


   if (USE_BLOCKS)
   {
      write_block[0] = CMD_1WRF;
      write_block[1] = rx_len;

      I2C_writeBlock(I2C_address, ADDR_CMD_REG, 2, write_block);

      // Use I2C_readBlockPoll to wait for result
      write_block[0] = ADDR_STATUS_REG;
      I2C_readBlockPoll(I2C_address, 1, write_block, 0xFF, STATUS_1WB, &status[0]);
      // fall through to manual polling
   }
   else
   {
      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write(CMD_1WRF, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      if (!I2C_write((uchar)rx_len, EXPECT_ACK))
      {
         I2C_stop();
         return FALSE;
      }
      I2C_stop();
      status[0] = STATUS_1WB;
   }
#endif
   // loop checking 1WB bit for completion of 1-Wire operation
   // abort if poll limit reached
   while ((status[0] & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
   {
      if (!DS2465_read(ADDR_STATUS_REG,status,1,false))
         return false;
   }

   // check for failure due to poll limit reached
   if (poll_count >= POLL_LIMIT)
   {
      // handle error
      // ...
      DS2465_reset();
      return false;
   }

   rt = DS2465_read(ADDR_SPAD,rx_buf,rx_len,false);

   // debug output
   if (owdebug)
   {
      for (i = 0; i < rx_len; i++)
         ;//printf("[%02X] ",rx_buf[i]);
   }

   // read out the data
   return rt;
}


//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire network
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : no device present
//
int OWFirst(void)
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = false;
   LastFamilyDiscrepancy = 0;

   return OWSearch();
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire network
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
int OWNext(void)
{
   // leave the search state alone
   return OWSearch();
}

//--------------------------------------------------------------------------
// Verify the device with the ROM number in ROM_NO buffer is present.
// Return TRUE  : device verified present
//        FALSE : device not present
//
int OWVerify(void)
{
   uchar rom_backup[8];
   int i,rslt,ld_backup,ldf_backup,lfd_backup;

   // keep a backup copy of the current state
   for (i = 0; i < 8; i++)
      rom_backup[i] = ROM_NO[i];
   ld_backup = LastDiscrepancy;
   ldf_backup = LastDeviceFlag;
   lfd_backup = LastFamilyDiscrepancy;

   // set search to find the same device
   LastDiscrepancy = 64;
   LastDeviceFlag = false;

   if (OWSearch())
   {
      // check if same device found
      rslt = true;
      for (i = 0; i < 8; i++)
      {
         if (rom_backup[i] != ROM_NO[i])
         {
            rslt = false;
            break;
         }
      }
   }
   else
     rslt = false;

   // restore the search state
   for (i = 0; i < 8; i++)
      ROM_NO[i] = rom_backup[i];
   LastDiscrepancy = ld_backup;
   LastDeviceFlag = ldf_backup;
   LastFamilyDiscrepancy = lfd_backup;

   // return the result of the verify
   return rslt;
}

//--------------------------------------------------------------------------
// Setup the search to find the device type 'family_code' on the next call
// to OWNext() if it is present.
//
void OWTargetSetup(uchar family_code)
{
   int i;

   // set the search state to find SearchFamily type devices
   ROM_NO[0] = family_code;
   for (i = 1; i < 8; i++)
      ROM_NO[i] = 0;
   LastDiscrepancy = 64;
   LastFamilyDiscrepancy = 0;
   LastDeviceFlag = false;
}

//--------------------------------------------------------------------------
// Setup the search to skip the current device type on the next call
// to OWNext().
//
void OWFamilySkipSetup(void)
{
   // set the Last discrepancy to last family discrepancy
   LastDiscrepancy = LastFamilyDiscrepancy;

   // clear the last family discrpepancy
   LastFamilyDiscrepancy = 0;

   // check for end of list
   if (LastDiscrepancy == 0)
      LastDeviceFlag = true;
}

//--------------------------------------------------------------------------
// The 'OWSearch' function does a general search.  This function
// continues from the previos search state. The search state
// can be reset by using the 'OWFirst' function.
// This function contains one parameter 'alarm_only'.
// When 'alarm_only' is TRUE (1) the find alarm command
// 0xEC is sent instead of the normal search command 0xF0.
// Using the find alarm command 0xEC will limit the search to only
// 1-Wire devices that are in an 'alarm' state.
//
// Returns:   TRUE (1) : when a 1-Wire device was found and it's
//                       Serial Number placed in the global ROM
//            FALSE (0): when no new device was found.  Either the
//                       last search was the last device or there
//                       are no devices on the 1-Wire Net.
//
int OWSearch(void)
{
   int id_bit_number;
   int last_zero, rom_byte_number, search_result;
   int id_bit, cmp_id_bit;
   uchar rom_byte_mask, search_direction, status;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = false;
   crc8 = 0;

   // if the last call was not the last one
   if (!LastDeviceFlag)
   {
      // 1-Wire reset
      if (!OWReset())
      {
         // reset the search
         LastDiscrepancy = 0;
         LastDeviceFlag = false;
         LastFamilyDiscrepancy = 0;
         return false;
      }

      // issue the search command
      OWWriteByte(0xF0);

      // loop to do the search
      do
      {
         // if this discrepancy if before the Last Discrepancy
         // on a previous next then pick the same as last time
         if (id_bit_number < LastDiscrepancy)
         {
            if ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0)
               search_direction = 1;
            else
               search_direction = 0;
         }
         else
         {
            // if equal to last pick 1, if not then pick 0
            if (id_bit_number == LastDiscrepancy)
               search_direction = 1;
            else
               search_direction = 0;
         }

         // Peform a triple operation on the DS2465 which will perform 2 read bits and 1 write bit
         status = DS2465_search_triplet(search_direction);

         // check bit results in status byte
         id_bit = ((status & STATUS_SBR) == STATUS_SBR);
         cmp_id_bit = ((status & STATUS_TSB) == STATUS_TSB);
         search_direction = ((status & STATUS_DIR) == STATUS_DIR) ? (uchar)1 : (uchar)0;

         // check for no devices on 1-wire
         if ((id_bit) && (cmp_id_bit))
            break;
         else
         {
            if ((!id_bit) && (!cmp_id_bit) && (search_direction == 0))
            {
               last_zero = id_bit_number;

               // check for Last discrepancy in family
               if (last_zero < 9)
                  LastFamilyDiscrepancy = last_zero;
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
               ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
               ROM_NO[rom_byte_number] &= (uchar)~rom_byte_mask;

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0)
            {
               calc_crc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
               rom_byte_number++;
               rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!((id_bit_number < 65) || (crc8 != 0)))
      {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         LastDiscrepancy = last_zero;

         // check for last device
         if (LastDiscrepancy == 0)
            LastDeviceFlag = true;

         search_result = true;
      }
   }

   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || (ROM_NO[0] == 0))
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = false;
      LastFamilyDiscrepancy = 0;
      search_result = false;
   }

   return search_result;
}

//--------------------------------------------------------------------------
// Use the DS2465 help command '1-Wire triplet' to perform one bit of a 1-Wire
// search. This command does two read bits and one write bit. The write bit
// is either the default direction (all device have same bit) or in case of
// a discripancy, the 'search_direction' parameter is used.
//
// Returns ?The DS2465 status byte result from the triplet command
//
uchar DS2465_search_triplet(int search_direction)
{
   uchar status;
   uchar write_block[3];
   int poll_count = 0, result;

   // 1-Wire Triplet (Case B)
   //   S AD,0 [A] 1WT [A] SS [A] Sr AD,1 [A] [Status] A [Status] A\ P
   //                                         \--------/
   //                           Repeat until 1WB bit has changed to 0
   //  [] indicates from slave
   //  SS indicates byte containing search direction bit value in msbit

#ifdef I2C_ON_IMX6S
    int cnt=0;

    gpioTools::I2C_start();

    write_block[cnt++] = ADDR_CMD_REG;
    write_block[cnt++] = CMD_1WT;
    write_block[cnt++] = (uchar)(search_direction ? 0x80 : 0x00);
    if(!gpioTools::I2C_Write(gpioTools::I2C_STATE_SIC,write_block, cnt))
    {
        gpioTools::I2C_stop();
        return false;
    }
    gpioTools::I2C_rep_start();

    // loop checking 1WB bit for completion of 1-Wire operation
    // abort if poll limit reached
    status = STATUS_1WB;
    while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
    {
       status = gpioTools::I2C_read(status & STATUS_1WB,&result);
       if (!result)
       {
          gpioTools::I2C_stop();
          return false;
       }
    }

    // one last read with NACK
    gpioTools::I2C_read(NACK,&result);
    if (!result)
    {
       gpioTools::I2C_stop();
       return false;
    }

    gpioTools::I2C_stop();

    // check for failure due to poll limit reached
    if (poll_count >= POLL_LIMIT)
    {
       // handle error
       // ...
       DS2465_reset();
       return 0;
    }

#else

   if (USE_BLOCKS)
   {
      write_block[0] = ADDR_CMD_REG;
      write_block[1] = CMD_1WT;
      write_block[2] = (search_direction ? 0x80 : 0x00);

      if (!I2C_readBlockPoll(I2C_address, 3, &write_block[0], 0xFF, STATUS_1WB, &status))
         return FALSE;
   }
   else
   {
      I2C_start();
      if (!I2C_write((uchar)(I2C_address | I2C_WRITE), EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      if (!I2C_write(ADDR_CMD_REG, EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      if (!I2C_write(CMD_1WT, EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      if (!I2C_write((uchar)(search_direction ? 0x80 : 0x00), EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }
      I2C_rep_start();
      if (!I2C_write((uchar)(I2C_address | I2C_READ), EXPECT_ACK))
      {
         I2C_stop();
         return 0;
      }

      // loop checking 1WB bit for completion of 1-Wire operation
      // abort if poll limit reached
      status = STATUS_1WB;
      while ((status & STATUS_1WB) && (poll_count++ < POLL_LIMIT))
      {
         status = I2C_read(status & STATUS_1WB,&result);
         if (!result)
         {
            I2C_stop();
            return FALSE;
         }
      }

      // one last read with NACK
      I2C_read(NACK,&result);
      if (!result)
      {
         I2C_stop();
         return FALSE;
      }

      I2C_stop();

      // check for failure due to poll limit reached
      if (poll_count >= POLL_LIMIT)
      {
         // handle error
         // ...
         DS2465_reset();
         return 0;
      }
   }
#endif

   // return status byte
   return status;
}

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current
// global 'crc8' value.
// Returns current global crc8 value
//
uchar calc_crc8(uchar data)
{
   int i;

   // See Application Note 27
   crc8 = crc8 ^ data;
   for (i = 0; i < 8; ++i)
   {
      if (crc8 & 1)
         crc8 = (crc8 >> 1) ^ 0x8c;
      else
         crc8 = (crc8 >> 1);
   }

   return crc8;
}

//--------------------------------------------------------------------------
// The 'OWReadROM' function does a Read-ROM.  This function
// uses the read-ROM function 33h to read a ROM number and verify CRC8.
//
// Returns:   TRUE (1) : OWReset successful and Serial Number placed
//                       in the global ROM, CRC8 valid
//            FALSE (0): OWReset did not have presence or CRC8 invalid
//
int OWReadROM(void)
{
   uchar buf[10];
   int i;

   if (OWReset() == 1)
   {
      OWWriteByte(0x33); // READ ROM

      // read the ROM
      OWReceiveBlock(buf,8);

      // verify CRC8
      crc8 = 0;
      for (i = 0; i < 8; i++)
         calc_crc8(buf[i]);

      if ((crc8 == 0) && (buf[1] != 0))
      {
         memcpy(ROM_NO,&buf[0],8);
         return true;
      }
   }

   return false;
}

//--------------------------------------------------------------------------
// The 'OWSkipROM' function does a skip-ROM.  This function
// uses the Skip-ROM function CCh.
//
// Returns:   TRUE (1) : OWReset successful and skip rom sent.
//            FALSE (0): OWReset did not have presence
//
int OWSkipROM(void)
{
   if (OWReset() == 1)
   {
      OWWriteByte(0xCC);
      return true;
   }

   return false;
}

//--------------------------------------------------------------------------
// The 'OWResume' function does a Resume command 0xA5.
//
// Returns:   TRUE (1) : OWReset successful and RESUME sent.
//            FALSE (0): OWReset did not have presence
//
int OWResume(void)
{
   if (OWReset() == 1)
   {
      OWWriteByte(0xA5);
      return true;
   }

   return false;
}

//--------------------------------------------------------------------------
// The 'OWOverdriveSkipROM' function does an Overdrive skip-ROM. Ignores
// result from standard speed OWReset().
//
// Returns:   TRUE (1) : OWReset and skip rom sent.
//            FALSE (0): Could not change to overdrive
//
int OWOverdriveSkipROM(void)
{
   OWSpeed(MODE_NORMAL);

   OWReset();
   OWWriteByte(0x3C);

   return (OWSpeed(MODE_OVERDRIVE) == MODE_OVERDRIVE);
}


//--------------------------------------------------------------------------
// The 'OWMatchROM' function does a Match-ROM using the global ROM_NO device
//
// Returns:   TRUE (1) : OWReset successful and match rom sent.
//            FALSE (0): OWReset did not have presence
//
int OWMatchROM(void)
{
   uchar buf[9];

   // use MatchROM
   if (OWReset())
   {
      buf[0] = 0x55;
      memcpy(&buf[1], &ROM_NO[0], 8);
      // send command and rom
      OWTransmitBlock(false,buf,9);
      return true;
   }
   else
      return false;
}

//--------------------------------------------------------------------------
// The 'OWOverdriveMatchROM' function does an overdrive Match-ROM using the
// global ROM_NO device
//
// Returns:   TRUE (1) : OWReset successful and match rom sent.
//            FALSE (0): OWReset did not have presence
//
int OWOverdriveMatchROM(void)
{
   // use overdrive MatchROM
   OWSpeed(MODE_NORMAL);
   if (OWReset())
   {
      OWWriteByte(0x69);
      OWSpeed(MODE_OVERDRIVE);
      // send ROM
      OWTransmitBlock(false,ROM_NO,8);
      return true;
   }
   else
      return false;
}

//---------------------------------------------------------------------------
//-------- Extended 1-Wire functions
//---------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Set the 1-Wire Net communication speed.
//
// 'new_speed' - new speed defined as
//                MODE_STANDARD   0x00
//                MODE_OVERDRIVE  0x01
//
// Returns:  current 1-Wire Net speed
//
int OWSpeed(int new_speed)
{
   // set the speed
   if (new_speed == MODE_OVERDRIVE)
      c1WS = CONFIG_1WS;
   else
      c1WS = false;

   // write the new config
   DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU));

   return new_speed;
}

//--------------------------------------------------------------------------
// Set the 1-Wire Net line level pull-up to normal. The DS2465 does only
// allows enabling strong pull-up on a bit or byte event. Consequently this
// function only allows the MODE_STANDARD argument. To enable strong pull-up
// use OWWriteBytePower or OWReadBitPower.
//
// 'new_level' - new level defined as
//                MODE_STANDARD     0x00
//
// Returns:  current 1-Wire Net level
//
int OWLevel(int new_level)
{
   // function only will turn back to non-strong pull-up
   if (new_level != MODE_STANDARD)
      return MODE_STRONG;

   // debug
   if (owdebug)
      ;//printf("<SP_OFF> ");

   // clear the strong pull-up bit in the global config state
   cSPU = false;

   // write the new config
   DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU));

   return MODE_STANDARD;
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net is the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.  After the
// 8 bits are sent change the level of the 1-Wire net.
//
// 'sendbyte' - 8 bits to send (least significant bit)
//
void OWWriteBytePower(uchar sendbyte)
{
   // set strong pull-up enable
   cSPU = CONFIG_SPU;

   // write the new config
   if (!DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU)))
      return;

   // perform write byte
   OWWriteByte(sendbyte);

   // debug
   if (owdebug)
      ;//printf("<SP_ON> ");
}

//--------------------------------------------------------------------------
// Read 8 bits of communication from the 1-Wire Net.  After the
// 8 bits are read then change the level of the 1-Wire net.
//
// Returns:  8 bits read from 1-Wire Net
//
uchar OWReadBytePower(void)
{
   uchar rslt;

   // set strong pull-up enable
   cSPU = CONFIG_SPU;

   // write the new config
   if (!DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU)))
      return false;

   // do the read byte
   rslt = OWReadByte();

   // debug
   if (owdebug)
      ;//printf("<SP_ON> ");

   return rslt;
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and verify that the
// response matches the 'applyPowerResponse' bit and apply power delivery
// to the 1-Wire net.  Note that some implementations may apply the power
// first and then turn it off if the response is incorrect.
//
// 'applyPowerResponse' - 1 bit response to check, if correct then start
//                        power delivery
//
// Returns:  TRUE: bit written and response correct, strong pullup now on
//           FALSE: response incorrect
//
int OWReadBitPower(int applyPowerResponse)
{
   uchar rdbit;

   // set strong pull-up enable
   cSPU = CONFIG_SPU;

   // write the new config
   if (!DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU)))
      return false;

   // perform read bit
   rdbit = OWReadBit();

   // debug
   if (owdebug)
      ;//printf("<SP_ON> ");

   // check if response was correct, if not then turn off strong pull-up
   if (rdbit != applyPowerResponse)
   {
      OWLevel(MODE_STANDARD);
      return false;
   }

   return true;
}

//--------------------------------------------------------------------------
// Power down 1-Wire using extended function
//
// Returns:  TRUE  successful
//           FALSE program voltage not available
//
int OWPowerDown(void)
{
   // debug
   if (owdebug)
      ;//printf("<POWER_DOWN> ");

   // set power down bit in the global config state
   cPDN = CONFIG_PDN;

   // write the new config
   return DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU));
}

//--------------------------------------------------------------------------
// Power up 1-Wire using extended function
//
// Returns:  TRUE  successful
//           FALSE failure during communication
//
int OWPowerUp(void)
{
   int rt;

   // debug
   if (owdebug)
      ;//printf("<POWER_UP> ");

   // clear power down bit in the global config state
   cPDN = false;

   // write the new config
   rt = DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU));

   // delay 2ms to allow units to power up
   gpioTools::msDelay(2);

   return rt;
}

//--------------------------------------------------------------------------
// APU enable or disable
//
// 'readflag' - 1 if reading current configuration
// 'apu_enable' - 1 to enable
//
// Returns:  TRUE  if write successful, or return configuration value if reading
//
int DS2465_OWConfigureAPU(int apu_enable)
{
   // debug
   if (owdebug)
   {
      if (apu_enable)
         ;//printf("<APU ENABLE>");
      else
         ;//printf("<APU DISABLE>");
   }

   // clear power down bit in the global config state
   cAPU = (apu_enable) ? CONFIG_APU : false;

   // write the new config
   return DS2465_write_config((uchar)(c1WS | cSPU | cPDN | cAPU));
}

//--------------------------------------------------------------------------
// Configure TRSTL timing
//
// 'readflag' - 1 if reading current configuration
// 'ovr' - overdrive value to write
// 'std' - standard value to write
//
// Returns:  TRUE  if write successful, or return configuration value if reading
//
int DS2465_OWConfigureTRSTL(int readflag, int ovr, int std)
{
   return DS2465_ow_config(readflag, ovr, std, ADDR_TRSTL_REG);
}

//--------------------------------------------------------------------------
// Configure TMSP timing
//
// 'readflag' - 1 if reading current configuration
// 'ovr' - overdrive value to write
// 'std' - standard value to write
//
// Returns:  TRUE  if write successful, or return configuration value
//            if reading
//
int DS2465_OWConfigureTMSP(int readflag, int ovr, int std)
{
   return DS2465_ow_config(readflag, ovr, std, ADDR_TMSP_REG);
}

//--------------------------------------------------------------------------
// Configure TW0L timing
//
// 'readflag' - 1 if reading current configuration
// 'ovr' - overdrive value to write
// 'std' - standard value to write
//
// Returns:  TRUE  if write successful, or return configuration value
//            if reading
//
int DS2465_OWConfigureTW0L(int readflag, int ovr, int std)
{
   return DS2465_ow_config(readflag, ovr, std, ADDR_TW0L_REG);
}

//--------------------------------------------------------------------------
// Configure TREC0 timing
//
// 'readflag' - 1 if reading current configuration
// 'ovr' - overdrive value to write
// 'std' - standard value to write
//
// Returns:  TRUE  if write successful, or return configuration value
//            if reading in 1/2 microseconds
//
int DS2465_OWConfigureTREC0(int readflag, int ovr, int std)
{
   return DS2465_ow_config(readflag, ovr, std, ADDR_TREC0_REG);
}

//--------------------------------------------------------------------------
// Configure TREC0 timing
//
// 'readflag' - 1 if reading current configuration
// 'value' - value to write
//
// Returns:  TRUE  if write successful, or return configuration value
//            if reading in 1/2 microseconds
//
int DS2465_OWConfigureRWPU(int readflag, int value)
{
   return DS2465_ow_config(readflag, 0, value, ADDR_RWPU_REG);
}

//--------------------------------------------------------------------------
// Configure overdrive TW1L timing
//
// 'readflag' - 1 if reading current configuration
// 'ovr' - overdrive value to write
//
// Returns:  TRUE  if write successful, or return configuration value
//            if reading
//
int DS2465_OWConfigureTW1L_Overdrive(int readflag, int ovr)
{
   return DS2465_ow_config(readflag, 0, ovr, ADDR_TW1L_REG);
}

//--------------------------------------------------------------------------
// Generic OW confiration function
//
// 'readflag' - 1 if reading current configuration
// 'ovr' - overdrive value to write
// 'std' - standard value to write
// 'addr' - configuration address
//
// Returns:  TRUE  if write successful, or return configuration value if reading
//
int DS2465_ow_config(int readflag, int ovr, int std, int addr)
{
   uchar buf[1];

   // check if reading only
   if (readflag)
   {
      // read the value
      if (DS2465_read(addr,buf,1,false))
         return buf[0];
      else
         return 0;
   }

   // convert and write value
   buf[0] = (ovr << 4) | std;
   return (DS2465_write_SRAM(addr, buf, 1));
}




