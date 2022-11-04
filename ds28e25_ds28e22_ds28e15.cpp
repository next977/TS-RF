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
// ds28e25_ds28e22_ds28e15.c - 1-Wire functions to authenticate and write to
//    the DS28E25, DS28E22, and DS28E15.
//

#include "sha256_software.h"
#include "1wire_ds2465.h"
#include "gpiotools.h"
#define DS28E25_DS28E22_DS28E15
#include "ds28e25_ds28e22_ds28e15.h"

#include <memory.h>

// function declarations
int writeBlock(int page, int block, uchar *data, int contflag);
int readPage(int page, uchar *rdbuf, int contflag);
int LoadSecret(int sn, uchar *secret, int lock);
int ComputeSecret(int sn, uchar *partial, int page_num, uchar *page_data, uchar *manid, int lock);
int WriteScratchpad(int sn, uchar *data);
int ComputeReadPageMAC(int sn, int page_num, uchar *challenge, uchar *mac, int anon);
int readAuthVerify(int sn, int page_num, uchar *challenge, uchar *page_data, uchar *manid, int skipread, int anon);
int ReadStatus(int personality, int allpages, int page_num, uchar *rdbuf);
int WriteBlockProtection(uchar prot, int contflag);
int WriteAuthBlockProtection(uchar new_value, uchar old_value, uchar *manid, int contflag);
int writeAuthBlock(int page, int block, uchar *new_data, uchar *old_data, uchar *manid, int contflag);
void SetSpecialMode(int enable, uchar *special_values);
void setDeviceSelectMode(int method);
int AuthVerify(int page_num, uchar *challenge, uchar *page_data, uchar *manid, uchar *mac, int anon);
int writeAuthBlockMAC(int page, int block, uchar *new_data, uchar *mac, int contflag);
int CalculateWriteAuthMAC256(int page, int block, uchar *new_data, uchar *old_data, uchar *manid, uchar *mac);

static unsigned short docrc16(unsigned short data);
static int DeviceSelect();

// External function
//extern void msDelay(int delay);

// misc state
static unsigned short CRC16;

static int select_method = SELECT_SKIP;
static int SPECIAL_MODE = 0;
static uchar SPECIAL_VALUES[2];

//-----------------------------------------------------------------------------
// ------ DS28E25, DS28E22, DS28E15 Functions
//-----------------------------------------------------------------------------

//--------------------------------------------------------------------------
//  Write a 4 byte memory block. The block location is selected by the
//  page number and offset blcok within the page. Multiple blocks can
//  be programmed without re-selecting the device using the continue flag.
//  This function does not use the Authenticated Write operation.
//
//  Parameters
//     page - page number where the block to write is located (0 to 15)
//     block - block number in page (0 to 7)
//     data - 4 byte buffer containing the data to write
//     contflag - Flag to indicate the write is continued from the last (=1)
//
//  Returns: TRUE - block written
//           FALSE - Failed to write block (no presence or invalid CRC16)
//
int writeBlock(int page, int block, uchar *data, int contflag)
{
   uchar buf[256],cs;
   int cnt, i, offset;

   cnt = 0;
   offset = 0;

   // check if not continuing a previous block write
   if (!contflag)
   {
      if (!DeviceSelect())
         return FALSE;

      buf[cnt++] = CMD_WRITE_MEMORY;
      buf[cnt++] = (block << 5) | page;   // address

      // Send command
      OWTransmitBlock(FALSE,&buf[0],2);

      // Read CRC
      OWReceiveBlock(&buf[cnt],2);
      cnt += 2;

      offset = cnt;
   }

   // add the data
   for (i = 0; i < 4; i++)
      buf[cnt++] = data[i];

   // Send data
   OWTransmitBlock(FALSE,data,4);

   // Read CRC
   OWReceiveBlock(&buf[cnt],2);
   cnt += 2;

   // check the first CRC16
   if (!contflag)
   {
      CRC16 = 0;
      for (i = 0; i < offset; i++)
         docrc16(buf[i]);

      if (CRC16 != 0xB001)
         return FALSE;
   }

   // check the second CRC16
   CRC16 = 0;
   for (i = offset; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // send release and strong pull-up
   OWWriteBytePower(0xAA);

   // now wait for the MAC computation.
   gpioTools::msDelay(EEPROM_WRITE_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // read the CS byte
   cs = OWReadByte();

   return (cs == 0xAA);
}

//--------------------------------------------------------------------------
//  Write a 4 byte memory block using an authenticated write (with MAC).
//  The block location is selected by the
//  page number and offset blcok within the page. Multiple blocks can
//  be programmed without re-selecting the device using the continue flag.
//  This function does not use the Authenticated Write operation.
//
//  Parameters
//     page - page number where the block to write is located (0 to 15)
//     block - block number in page (0 to 7)
//     new_data - 4 byte buffer containing the data to write
//     old_data - 4 byte buffer containing the data to write
//     manid - 2 byte buffer containing the manufacturer ID (general device: 00h,00h)
//     contflag - Flag to indicate the write is continued from the last (=1)
//
//  Returns: TRUE - block written
//           FALSE - Failed to write block (no presence or invalid CRC16)
//
int writeAuthBlock(int page, int block, uchar *new_data, uchar *old_data, uchar *manid, int contflag)
{
   uchar buf[256],cs;
   int cnt, i, offset;

   cnt = 0;
   offset = 0;

   // check if not continuing a previous block write
   if (!contflag)
   {
      if (!DeviceSelect())
         return FALSE;

      buf[cnt++] = CMD_WRITE_AUTH_MEMORY;
      buf[cnt++] = (block << 5) | page;   // address

      // Send command
      OWTransmitBlock(FALSE,&buf[0],2);

      // Read CRC
      OWReceiveBlock(&buf[cnt],2);
      cnt += 2;

      offset = cnt;
   }

   // add the data
   for (i = 0; i < 4; i++)
      buf[cnt++] = new_data[i];

   // Send data
   OWTransmitBlock(FALSE,new_data,4);

   // read first CRC byte
   buf[cnt++] = OWReadByte();

   // read the last CRC and enable power
   buf[cnt++] = OWReadBytePower();

   // now wait for the MAC computation.
   gpioTools::msDelay(SHA_COMPUTATION_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // check the first CRC16
   if (!contflag)
   {
      CRC16 = 0;
      for (i = 0; i < offset; i++)
         docrc16(buf[i]);

      if (CRC16 != 0xB001)
         return FALSE;
   }

   // check the second CRC16
   CRC16 = 0;

   // DS28E25/DS28E22, crc gets calculagted with CS byte
   if (((ROM_NO[0] & 0x7F) == DS28E25_FAMILY) || ((ROM_NO[0] & 0x7F) == DS28E22_FAMILY))
   {
      if (contflag)
         docrc16(0xAA);
   }

   for (i = offset; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // compute the mac
   if (SPECIAL_MODE)
   {
      if (!CalculateWriteAuthMAC256(page, block, new_data, old_data, SPECIAL_VALUES, &buf[0]))
         return FALSE;
   }
   else
   {
      if (!CalculateWriteAuthMAC256(page, block, new_data, old_data, manid, &buf[0]))
         return FALSE;
   }

   // transmit MAC as a block
   cnt=0;
   OWTransmitBlock(FALSE, buf, 32);

   // calculate CRC on MAC
   CRC16 = 0;
   for (i = 0; i < 32; i++)
      docrc16(buf[i]);

   // append read of CRC16 and CS byte
   OWReceiveBlock(&buf[0],3);
   cnt = 3;

   // ckeck CRC16
   for (i = 0; i < (cnt-1); i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // check CS
   if (buf[cnt-1] != 0xAA)
      return FALSE;

   // send release and strong pull-up
   OWWriteBytePower(0xAA);

   // now wait for the MAC computation.
   gpioTools::msDelay(EEPROM_WRITE_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // read the CS byte
   cs = OWReadByte();

   return (cs == 0xAA);
}

//--------------------------------------------------------------------------
//  Write a 4 byte memory block using an authenticated write (with MAC).
//  The MAC must be pre-calculated.
//
//  Parameters
//     page - page number where the block to write is located (0 to 15)
//     block - block number in page (0 to 7)
//     new_data - 4 byte buffer containing the data to write
//     mac - mac to use for the write
//     contflag - Flag to indicate the write is continued from the last (=1)
//
//  Returns: TRUE - block written
//           FALSE - Failed to write block (no presence or invalid CRC16)
//
int writeAuthBlockMAC(int page, int block, uchar *new_data, uchar *mac, int contflag)
{
   uchar buf[256],cs;
   int cnt, i, offset;

   cnt = 0;
   offset = 0;

   // check if not continuing a previous block write
   if (!contflag)
   {
      if (!DeviceSelect())
         return FALSE;

      buf[cnt++] = CMD_WRITE_AUTH_MEMORY;
      buf[cnt++] = (block << 5) | page;   // address

      // Send command
      OWTransmitBlock(FALSE,&buf[0],2);

      // Read CRC
      OWReceiveBlock(&buf[cnt],2);
      cnt += 2;

      offset = cnt;
   }

   // add the data
   for (i = 0; i < 4; i++)
      buf[cnt++] = new_data[i];

   // Send data
   OWTransmitBlock(FALSE,new_data,4);

   // read first CRC byte
   buf[cnt++] = OWReadByte();

   // read the last CRC and enable power
   buf[cnt++] = OWReadBytePower();

      // now wait for the MAC computation.
   gpioTools::msDelay(SHA_COMPUTATION_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // check the first CRC16
   if (!contflag)
   {
      CRC16 = 0;
      for (i = 0; i < offset; i++)
         docrc16(buf[i]);

      if (CRC16 != 0xB001)
         return FALSE;
   }

   // check the second CRC16
   CRC16 = 0;

   // DS28E25/DS28E22, crc gets calculagted with CS byte
   if (((ROM_NO[0] & 0x7F) == DS28E25_FAMILY) || ((ROM_NO[0] & 0x7F) == DS28E22_FAMILY))
   {
      if (contflag)
         docrc16(0xAA);
   }

   for (i = offset; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // transmit MAC as a block
   OWTransmitBlock(FALSE, mac, 32);

   // calculate CRC on MAC
   CRC16 = 0;
   for (i = 0; i < 32; i++)
      docrc16(mac[i]);

   // append read of CRC16 and CS byte
   OWReceiveBlock(&buf[0],3);
   cnt = 3;

   // ckeck CRC16
   for (i = 0; i < (cnt-1); i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // check CS
   if (buf[cnt-1] != 0xAA)
      return FALSE;

   // send release and strong pull-up
   OWWriteBytePower(0xAA);

   // now wait for the MAC computation.
   gpioTools::msDelay(EEPROM_WRITE_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // read the CS byte
   cs = OWReadByte();

   return (cs == 0xAA);
}

//--------------------------------------------------------------------------
//  Read page and verify CRC. Multiple pages can
//  be read without re-selecting the device using the continue flag.
//
//  Parameters
//     page - page number where the block to write is located (0 to 15)
//     rdbuf - 32 byte buffer to contain the data to read
//     contflag - Flag to indicate the write is continued from the last (=1)
//
//  Returns: TRUE - block read and verified CRC
//           FALSE - Failed to write block (no presence or invalid CRC16)
//
int readPage(int page, uchar *rdbuf, int contflag)
{
   uchar buf[256];
   int cnt, i, offset;

   cnt = 0;
   offset = 0;

   // check if not continuing a previous block write
   if (!contflag)
   {
      if (!DeviceSelect())
         return FALSE;

      buf[cnt++] = CMD_READ_MEMORY;
      buf[cnt++] = page;   // address

      // Send command
      OWTransmitBlock(FALSE,&buf[0],2);

      // Read CRC
      OWReceiveBlock(&buf[cnt],2);
      cnt += 2;

      offset = cnt;
   }

   // read data and CRC16
   OWReceiveBlock(&buf[cnt],34);
   cnt+=34;

   // check the first CRC16
   if (!contflag)
   {
      CRC16 = 0;
      for (i = 0; i < offset; i++)
         docrc16(buf[i]);

      if (CRC16 != 0xB001)
         return FALSE;
   }

   // check the second CRC16
   CRC16 = 0;
   for (i = offset; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // copy the data to the read buffer
   memcpy(rdbuf,&buf[offset],32);

   return TRUE;
}

//----------------------------------------------------------------------
// Load first secret operation on the DS28E25/DS28E22/DS28E15.
//
// 'sn'        - secret number 0 or 1
// 'secret'    - secret to load (32 bytes)
// 'lock'      - option to lock the secret after the load (lock = 1)
//
// Return: TRUE - load complete
//         FALSE - error during load, device not present
//
int LoadSecret(int sn, uchar *secret, int lock)
{
   uchar buf[256],cs;
   int cnt=0, i;

   // write the new secret to the scratchpad
   if (!WriteScratchpad(sn,secret))
      return FALSE;

   // select device for write
   if (!DeviceSelect())
      return FALSE;

   buf[cnt++] = CMD_LOAD_LOCK_SECRET;
   buf[cnt++] = (lock) ? 0xE0 : 0x00;  // lock flag

   // Send command
   OWTransmitBlock(FALSE,&buf[0],2);

   // Read CRC
   OWReceiveBlock(&buf[cnt],2);
   cnt += 2;

   // check CRC16
   CRC16 = 0;
   for (i = 0; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // send release and strong pull-up
   OWWriteBytePower(0xAA);

   // now wait for the MAC computation.
   gpioTools::msDelay(SECRET_EEPROM_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // read the CS byte
   cs = OWReadByte();

   return (cs == 0xAA);
}

//----------------------------------------------------------------------
// Compute secret operation on the DS28E25/DS28E22/DS28E15.
//
// 'sn'        - secret number 0 or 1
// 'partial'   - partial secret to load (32 bytes)
// 'pagedata'  - page data to compute (32 bytes)
// 'lock'      - option to lock the secret after the load (lock = 1)
//
// Return: TRUE - load complete
//         FALSE - error during load, device not present
//
int ComputeSecret(int sn, uchar *partial, int page_num, uchar *page_data, uchar *manid, int lock)
{
   uchar buf[256],cs;
   int cnt=0, i;

   // write the new secret to the scratchpad
   if (!WriteScratchpad(sn,partial))
      return FALSE;

   // select device for write
   if (!DeviceSelect())
      return FALSE;

   buf[cnt++] = CMD_COMPUTE_LOCK_SECRET;
   buf[cnt++] = (lock) ? (0xE0 | page_num) : page_num;  // lock flag

   // Send command
   OWTransmitBlock(FALSE,&buf[0],2);

   // Read CRC
   OWReceiveBlock(&buf[cnt],2);
   cnt += 2;

   // check CRC16
   CRC16 = 0;
   for (i = 0; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // send release and strong pull-up
   OWWriteBytePower(0xAA);

   // now wait for the MAC computation.
   gpioTools::msDelay(SHA_COMPUTATION_DELAY * 2 + SECRET_EEPROM_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // read the CS byte
   cs = OWReadByte();

   return (cs == 0xAA);
}

//----------------------------------------------------------------------
// Set or clear special mode flag
//
// 'enable'      - '1' to enable special mode or '0' to clear
//
void SetSpecialMode(int enable, uchar *special_values)
{
   SPECIAL_MODE = enable;
   SPECIAL_VALUES[0] = special_values[0];
   SPECIAL_VALUES[1] = special_values[1];
}

//----------------------------------------------------------------------
// Write the scratchpad (challenge or secret)
//
// 'sn'        - secret number 0 or 1
// 'data'      - data to write to the scratchpad (32 bytes)
//
// Return: TRUE - select complete
//         FALSE - error during select, device not present
//
int WriteScratchpad(int sn, uchar *data)
{
   uchar buf[256];
   int cnt=0, i, offset;

   // select device for write
   if (!DeviceSelect())
      return FALSE;

   buf[cnt++] = CMD_SELECT_SECRET;
   if (((ROM_NO[0] & 0x7F) == DS28E25_FAMILY) || ((ROM_NO[0] & 0x7F) == DS28E22_FAMILY))
      buf[cnt++] = sn ? 0x40 : 0x20; // select secret
   else
      buf[cnt++] = SPECIAL_MODE ? 0xF0 : 0x00;

   // Send command
   OWTransmitBlock(FALSE,&buf[0],2);

   // Read CRC
   OWReceiveBlock(&buf[cnt],2);
   cnt += 2;

   offset = cnt;

   // add the data
   memcpy(&buf[cnt], data, 32);
   cnt+=32;

   // Send the data
   OWTransmitBlock(FALSE, data, 32);

   // Read CRC
   OWReceiveBlock(&buf[cnt],2);
   cnt += 2;

   // check first CRC16
   CRC16 = 0;
   for (i = 0; i < offset; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // check the second CRC16
   CRC16 = 0;
   for (i = offset; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   return TRUE;
}

//--------------------------------------------------------------------------
//  Do Compute Page MAC command and return MAC. Optionally do
//  annonymous mode (anon != 0).
//
//  Parameters
//     sn  - secret number 0 or 1
//     page_num - page number to read 0 - 16
//     challange - 32 byte buffer containing the challenge
//     mac - 32 byte buffer for page data read
//     anon - Flag to indicate Annonymous mode if (anon != 0)
//
//  Returns: TRUE - page read has correct MAC
//           FALSE - Failed to read page or incorrect MAC
//
int ComputeReadPageMAC(int sn, int page_num, uchar *challenge, uchar *mac, int anon)
{
   uchar buf[256],cs;
   int cnt=0, i;

   // write the new secret to the scratchpad
   if (!WriteScratchpad(sn,challenge))
      return FALSE;

   // select device for write
   if (!DeviceSelect())
      return FALSE;

   buf[cnt++] = CMD_COMPUTE_PAGEMAC;
   buf[cnt++] = ((anon) ? 0xE0 : 0x00) | page_num;

   // Send command
   OWTransmitBlock(FALSE,&buf[0],2);

   // read first CRC byte
   buf[cnt++] = OWReadByte();

   // read the last CRC and enable
   buf[cnt++] = OWReadBytePower();

   // now wait for the MAC computation.
   gpioTools::msDelay(SHA_COMPUTATION_DELAY * 2);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // check CRC16
   CRC16 = 0;
   for (i = 0; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // read the CS byte
   cs = OWReadByte();
   if (cs != 0xAA)
      return FALSE;

   // read the MAC and CRC
   OWReceiveBlock(&buf[0],34);

   // check CRC16
   CRC16 = 0;
   for (i = 0; i < 34; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // copy MAC to return buffer
   memcpy(mac, buf, 32);

   return TRUE;
}

//--------------------------------------------------------------------------
//  Do Read Athenticated Page command and verify MAC. Optionally do
//  annonymous mode (anon != 0).
//
//  Parameters
//     sn  - secret number 0 or 1
//     page_num - page number to read 0 - 16
//     challange - 32 byte buffer containing the challenge
//     mac - 32 byte buffer for mac read
//     page_data - 32 byte buffer to contain the data to read
//     manid - 2 byte buffer containing the manufacturer ID (general device: 00h,00h)
//     skipread - Skip the read page and use the provided data in the 'page_data' buffer
//     anon - Flag to indicate Annonymous mode if (anon != 0)
//
//  Returns: TRUE - page read has correct MAC
//           FALSE - Failed to read page or incorrect MAC
//
int readAuthVerify(int sn, int page_num, uchar *challenge, uchar *page_data, uchar *manid, int skipread, int anon)
{
   uchar mac[32];
   uchar MT[128];

   // check to see if we skip the read (use page_data)
   if (!skipread)
   {
      // read the page to get data
      if (!readPage(page_num, page_data, FALSE))
         return FALSE;
   }

   // have device compute mac
   if (!ComputeReadPageMAC(sn, page_num, challenge, mac, anon))
      return FALSE;

   // create buffer to compute and verify mac

   // clear
   memset(MT,0,128);

   // insert page data
   memcpy(&MT[0],page_data,32);

   // insert challenge
   memcpy(&MT[32],challenge,32);

   // insert ROM number or FF
   if (anon)
      memset(&MT[96],0xFF,8);
   else
      memcpy(&MT[96],ROM_NO,8);

   MT[106] = page_num;

   if (SPECIAL_MODE)
   {
      MT[105] = SPECIAL_VALUES[0];
      MT[104] = SPECIAL_VALUES[1];
   }
   else
   {
      MT[105] = manid[0];
      MT[104] = manid[1];
   }

   return VerifyMAC256(MT, 119, mac);
}

//--------------------------------------------------------------------------
//  Verify provided MAC and page data. Optionally do
//  annonymous mode (anon != 0).
//
//  Parameters
//     page_num - page number to read 0 - 16
//     challange - 32 byte buffer containing the challenge
//     page_data - 32 byte buffer to contain the data read
//     manid - 2 byte buffer containing the manufacturer ID (general device: 00h,00h)
//     mac - 32 byte buffer of mac read
//     anon - Flag to indicate Annonymous mode if (anon != 0)
//
//  Returns: TRUE - page read has correct MAC
//           FALSE - Failed to read page or incorrect MAC
//
int AuthVerify(int page_num, uchar *challenge, uchar *page_data, uchar *manid, uchar *mac, int anon)
{
   uchar MT[128];

   // create buffer to compute and verify mac

   // clear
   memset(MT,0,128);

   // insert page data
   memcpy(&MT[0],page_data,32);

   // insert challenge
   memcpy(&MT[32],challenge,32);

   // insert ROM number or FF
   if (anon)
      memset(&MT[96],0xFF,8);
   else
      memcpy(&MT[96],ROM_NO,8);

   MT[106] = page_num;

   if (SPECIAL_MODE)
   {
      MT[105] = SPECIAL_VALUES[0];
      MT[104] = SPECIAL_VALUES[1];
   }
   else
   {
      MT[105] = manid[0];
      MT[104] = manid[1];
   }

   return VerifyMAC256(MT, 119, mac);
}

//--------------------------------------------------------------------------
//  Read status bytes, either personality or page protection.
//
//  Parameters
//     personality - flag to indicate the read is the 4 personality bytes (1)
//                  or page page protection (0)
//     allpages - flag to indicate if just one page (0) or all (1) page protection
//                 bytes.
//     page_num - page number if reading protection 0 to 1
//     rdbuf - 16 byte buffer personality bytes (length 4) or page protection
//            (length 1 or 16)
//
//  Returns: TRUE - status read
//           FALSE - Failed to read status
//
int ReadStatus(int personality, int allpages, int page_num, uchar *rdbuf)
{
   uchar buf[64]; //256
   int cnt, i, offset,rdnum;

   cnt = 0;
   offset = 0;

   if (!DeviceSelect())
      return FALSE;

   buf[cnt++] = CMD_READ_STATUS;
   if (personality)
      buf[cnt++] = 0xE0;
   else if (!allpages)
      buf[cnt++] = page_num;
   else
      buf[cnt++] = 0;

   // send the command
   OWTransmitBlock(FALSE,&buf[0],2);

   offset = cnt + 2;

   // adjust data length
   if (((ROM_NO[0] & 0x7F) == DS28E25_FAMILY) || ((ROM_NO[0] & 0x7F) == DS28E22_FAMILY))
   {
      if (personality)
         rdnum = 8;
      else if (allpages)
         rdnum = 20;
      else
         rdnum = 5;
   }
   else
   {
      if ((personality) || (allpages))
         rdnum = 8;
      else
         rdnum = 5;
   }

   // Read the bytes
   OWReceiveBlock(&buf[cnt],rdnum);
   cnt += rdnum;
#if 0
   // check the first CRC16
   CRC16 = 0;
   for (i = 0; i < offset; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   if (((((ROM_NO[0] & 0x7F) == DS28E25_FAMILY) || ((ROM_NO[0] & 0x7F) == DS28E22_FAMILY))
        && (allpages || (page_num == 15))) ||
       (personality || allpages || (page_num == 1)))
   {
      // check the second CRC16
      CRC16 = 0;
      for (i = offset; i < cnt; i++)
         docrc16(buf[i]);

      if (CRC16 != 0xB001)
         return FALSE;
   }
#endif
   // copy the data to the read buffer
   memcpy(rdbuf,&buf[offset],rdnum-4);

   return TRUE;
}

//--------------------------------------------------------------------------
//  Write page protection byte.
//
//  Parameters
//     block - block number (0 to 7) which covers two pages each
//     prot - protection byte
//     contflag - Flag to indicate the write is continued from the last (=1)
//
//  Returns: TRUE - protection written
//           FALSE - Failed to set protection
//
int WriteBlockProtection(uchar prot, int contflag)
{
   uchar buf[256],cs;
   int cnt=0, i;

   // check if not continuing a previous block write
   if (!contflag)
   {
      // select device for write
      if (!DeviceSelect())
         return FALSE;

      buf[cnt++] = CMD_WRITE_BLOCK_PROTECT;
   }

   // compute parameter byte
   buf[cnt++] = prot;

   OWTransmitBlock(FALSE,&buf[0],cnt);

   // Read CRC
   OWReceiveBlock(&buf[cnt],2);
   cnt += 2;

   // check CRC16
   CRC16 = 0;
   for (i = 0; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // DATASHEET_CORRECTION, on continue need second release byte
   if (contflag)
      OWWriteByte(0xAA);

   // sent release
   OWWriteBytePower(0xAA);

   // now wait for programming
   gpioTools::msDelay(EEPROM_WRITE_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // read the CS byte
   cs = OWReadByte();

   return (cs == 0xAA);
}

//--------------------------------------------------------------------------
//  Write page protection byte.
//
//  Parameters
//     block - block number (0 to 7) which covers two pages each
//     new_value - new protection byte
//     old_value - old protection byte
//     manid - manufacturer ID
//     contflag - Flag to indicate the write is continued from the last (=1)
//
//  Returns: TRUE - protection written
//           FALSE - Failed to set protection
//
int WriteAuthBlockProtection(uchar new_value, uchar old_value, uchar *manid, int contflag)
{
   uchar buf[256],cs,MT[64];
   int cnt=0, i;

   // select device for write
   if (!DeviceSelect())
      return FALSE;

   buf[cnt++] = CMD_WRITE_AUTH_PROTECT;
   buf[cnt++] = new_value;

   // Send command
   OWTransmitBlock(FALSE,&buf[0],2);

   // read first CRC byte
   buf[cnt++] = OWReadByte();

   // read the last CRC and enable
   buf[cnt++] = OWReadBytePower();

   // now wait for the MAC computation.
   gpioTools::msDelay(SHA_COMPUTATION_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // check CRC16
   CRC16 = 0;
   for (i = 0; i < cnt; i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // calculate MAC
   // clear
   memset(MT,0,64);

   // insert ROM number
   memcpy(&MT[32],ROM_NO,8);

   // instert block and page
   MT[43] = 0;
   MT[42] = new_value & 0x0F;

   // check on special mode
   if (SPECIAL_MODE)
   {
      MT[41] = SPECIAL_VALUES[0];
      MT[40] = SPECIAL_VALUES[1];
   }
   else
   {
      MT[41] = manid[0];
      MT[40] = manid[1];
   }

   // old data
   MT[44] = (old_value & PROT_BIT_AUTHWRITE) ? 0x01 : 0x00;
   MT[45] = (old_value & PROT_BIT_EPROM) ? 0x01 : 0x00;
   MT[46] = (old_value & PROT_BIT_WRITE) ? 0x01 : 0x00;
   MT[47] = (old_value & PROT_BIT_READ) ? 0x01 : 0x00;
   // new data
   MT[48] = (new_value & PROT_BIT_AUTHWRITE) ? 0x01 : 0x00;
   MT[49] = (new_value & PROT_BIT_EPROM) ? 0x01 : 0x00;
   MT[50] = (new_value & PROT_BIT_WRITE) ? 0x01 : 0x00;
   MT[51] = (new_value & PROT_BIT_READ) ? 0x01 : 0x00;

   // compute the mac
   ComputeMAC256(MT, 55, &buf[0]);
   cnt = 32;

   // send the MAC
   OWTransmitBlock(FALSE,&buf[0],32);

   // Read CRC and CS byte
   OWReceiveBlock(&buf[cnt],3);
   cnt += 3;

   // ckeck CRC16
   CRC16 = 0;
   for (i = 0; i < (cnt-1); i++)
      docrc16(buf[i]);

   if (CRC16 != 0xB001)
      return FALSE;

   // check CS
   if (buf[cnt-1] != 0xAA)
      return FALSE;

   // send release and strong pull-up
   // DATASHEET_CORRECTION - last bit in release is a read-zero so don't check echo of write byte
   OWWriteBytePower(0xAA);

   // now wait for the MAC computation.
   gpioTools::msDelay(EEPROM_WRITE_DELAY);

   // disable strong pullup
   OWLevel(MODE_NORMAL);

   // read the CS byte
   cs = OWReadByte();

   return (cs == 0xAA);
}

//--------------------------------------------------------------------------
//  Compute MAC to write a 4 byte memory block using an authenticated
//  write.
//
//  Parameters
//     page - page number where the block to write is located (0 to 15)
//     block - block number in page (0 to 7)
//     new_data - 4 byte buffer containing the data to write
//     old_data - 4 byte buffer containing the data to write
//     manid - 2 byte buffer containing the manufacturer ID (general device: 00h,00h)
//     mac - buffer to put the calculated mac into
//
//  Returns: TRUE - mac calculated
//           FALSE - Failed to calculate
//
int CalculateWriteAuthMAC256(int page, int block, uchar *new_data, uchar *old_data, uchar *manid, uchar *mac)
{
   uchar MT[64];

   // calculate MAC
   // clear
   memset(MT,0,64);

   // insert ROM number
   memcpy(&MT[32],ROM_NO,8);

   MT[43] = block;
   MT[42] = page;
   MT[41] = manid[0];
   MT[40] = manid[1];

   // insert old data
   memcpy(&MT[44],old_data,4);

   // insert new data
   memcpy(&MT[48],new_data,4);

   // compute the mac
   return ComputeMAC256(MT, 55, &mac[0]);
}

//--------------------------------------------------------------------------
//  Select the DS28E01 in the global ROM_NO
//
static int DeviceSelect()
{
   switch(select_method)
   {
   case SELECT_SKIP:
      return OWSkipROM();
   case SELECT_RESUME:
      return OWResume();
   case SELECT_MATCH:
      return OWMatchROM();
   case SELECT_ODMATCH:
      return OWOverdriveMatchROM();
   case SELECT_SEARCH:
      return OWFirst();
   case SELECT_READROM:
      return OWReadROM();
   case SELECT_ODSKIP:
      return OWOverdriveSkipROM();
   };

   return 0;
}

//--------------------------------------------------------------------------
//  Set the method to select the device
//
void setDeviceSelectMode(int method)
{
   select_method = method;
}

//--------------------------------------------------------------------------
// Calculate a new CRC16 from the input data shorteger.  Return the current
// CRC16 and also update the global variable CRC16.
//
static short oddparity[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

static unsigned short docrc16(unsigned short data)
{
   data = (data ^ (CRC16 & 0xff)) & 0xff;
   CRC16 >>= 8;

   if (oddparity[data & 0xf] ^ oddparity[data >> 4])
     CRC16 ^= 0xc001;

   data <<= 6;
   CRC16  ^= data;
   data <<= 1;
   CRC16   ^= data;

   return CRC16;
}
