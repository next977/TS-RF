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
// ds28e25_ds28e22_ds28e15.h - Include file for ds28e25_ds28e22_ds28e15.c
//

// misc constants
#define TRUE    1
#define FALSE   0

// delay durations

#ifdef LOW_VOLTAGE
#define SHA_COMPUTATION_DELAY    4
#define EEPROM_WRITE_DELAY       15
#define SECRET_EEPROM_DELAY      200
#else
#define SHA_COMPUTATION_DELAY    3
#define EEPROM_WRITE_DELAY       10
#define SECRET_EEPROM_DELAY      90
#endif

// 1-Wire commands
#define CMD_WRITE_MEMORY         0x55
#define CMD_READ_MEMORY          0xF0
#define CMD_LOAD_LOCK_SECRET     0x33
#define CMD_COMPUTE_LOCK_SECRET  0x3C
#define CMD_SELECT_SECRET        0x0F
#define CMD_COMPUTE_PAGEMAC      0xA5
#define CMD_READ_STATUS          0xAA
#define CMD_WRITE_BLOCK_PROTECT  0xC3
#define CMD_WRITE_AUTH_MEMORY    0x5A
#define CMD_WRITE_AUTH_PROTECT   0xCC
#define CMD_PIO_READ             0xDD
#define CMD_PIO_WRITE            0x96

#define BLOCK_READ_PROTECT       0x80
#define BLOCK_WRITE_PROTECT      0x40
#define BLOCK_EPROM_PROTECT      0x20
#define BLOCK_WRITE_AUTH_PROTECT 0x10

#define ROM_CMD_SKIP             0x3C
#define ROM_CMD_RESUME           0xA5

#define SELECT_SKIP     0
#define SELECT_RESUME   1
#define SELECT_MATCH    2
#define SELECT_ODMATCH  3
#define SELECT_SEARCH   4
#define SELECT_READROM  5
#define SELECT_ODSKIP   6

#define PROT_BIT_AUTHWRITE 0x10
#define PROT_BIT_EPROM     0x20
#define PROT_BIT_WRITE     0x40
#define PROT_BIT_READ      0x80

#define DS28E25_FAMILY   0x47
#define DS28E22_FAMILY   0x48
#define DS28E15_FAMILY   0x17

#define DS28E25_PAGES    16
#define DS28E22_PAGES    8
#define DS28E15_PAGES    2


#ifndef uchar
   typedef unsigned char uchar;
#endif

#ifndef DS28E25_DS28E22_DS28E15

   extern int writeBlock(int page, int block, uchar *data, int contflag);
   extern int readPage(int page, uchar *rdbuf, int contflag);
   extern int LoadSecret(int sn, uchar *secret, int lock);
   extern int ComputeSecret(int sn, uchar *partial, int page_num, uchar *page_data, uchar *manid, int lock);
   extern int WriteScratchpad(int sn, uchar *data);
   extern int ComputeReadPageMAC(int sn, int page_num, uchar *challenge, uchar *mac, int anon);
   extern int readAuthVerify(int sn, int page_num, uchar *challenge, uchar *page_data, uchar *manid, int skipread, int anon);
   extern int ReadStatus(int personality, int allpages, int page_num, uchar *rdbuf);

   extern int WriteBlockProtection(uchar prot, int contflag);
   extern int WriteAuthBlockProtection(uchar new_value, uchar old_value, uchar *manid, int contflag);

   extern int writeAuthBlock(int page, int block, uchar *new_data, uchar *old_data, uchar *manid, int contflag);
   extern void SetSpecialMode(int enable, uchar *special_values);
   extern void setDeviceSelectMode(int method);

   extern int AuthVerify(int page_num, uchar *challenge, uchar *page_data, uchar *manid, uchar *mac, int anon);
   extern int writeAuthBlockMAC(int page, int block, uchar *new_data, uchar *mac, int contflag);
   extern int CalculateWriteAuthMAC256(int page, int block, uchar *new_data, uchar *old_data, uchar *manid, uchar *mac);
   extern int CalculateNextSecret256(uchar* binding, uchar* partial, int page_num, uchar* man_id);

#endif


