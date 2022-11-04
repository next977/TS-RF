
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <QDebug>
#include "gpiotools.h"
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdarg.h>
#include <termio.h>
#include <sys/timeb.h>
#include <signal.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

//<--------------------RTC & Volume----------------------->
//#define FEATURE_ADS8343
#define FEATURE_ADS8341

#define USING_SMBUS         true

#define MODE_WRITEBLOCK     1
#define MODE_READBLOCK      2
#define MODE_WRITEREADBLOCK 3
#define MODE_WRITEPOLL      4



// defines the data direction (reading from I2C device) in I2C_start(),I2C_rep_start()
#define I2C_READ    1

// defines the data direction (writing to I2C device) in I2C_start(),I2C_rep_start()
#define I2C_WRITE   0

// Flags on I2C_read
#define ACK    1
#define NACK   0

#define EXPECT_ACK    1
#define EXPECT_NACK   0

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
#define ADDR_CMD_REG    0x60 //
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

#define mSleep(n) usleep(n*1000)
//<------------------------------------------------------>

using namespace std;

namespace gpioTools {

#define DECODE_MODE		0x09
#define INTENSITY_CONTROL	0x0A
#define SCAN_LIMIT			0x0B
#define SHUTDOWN			0X0C
#define FEATURE				0X0E
#define DISPLAY_TEST		0X0F

static const uint32 IOMUXC = 0x020E0000;
static const uint32 IOMUXC_SW_MUX_CTL_PAD_SD4_DATA0 = 0x020E031C;
static const uint32 GPIO_BASE = 0x0209C000;

//GPIO Memory Map
static const uint32 GPIO1_DR = 0x0209C000;
static const uint32 GPIO2_DR = 0x020A0000;
static const uint32 GPIO3_DR = 0x020A4000;
static const uint32 GPIO4_DR = 0x020A8000;
static const uint32 GPIO5_DR = 0x020AC000;
static const uint32 GPIO6_DR = 0x020B0000;


static volatile uint32 *gpio1_address;
static volatile uint32 *gpio2_address;
static volatile uint32 *gpio3_address;
static volatile uint32 *gpio4_address;
static volatile uint32 *gpio5_address;
static volatile uint32 *gpio6_address;

static const uint32 GPIO_OUTPUT = 1;
static const uint32 GPIO_INPUT = 0;

static int32 _spifd[ALL_SPI_DEVICES];
static uint8 _mode[ALL_SPI_DEVICES];
static uint8 _bits_per_word[ALL_SPI_DEVICES];
static uint32 _speed[ALL_SPI_DEVICES];

static volatile uint8 _tx_buf[ALL_SPI_DEVICES][10];
static volatile uint8 _rx_buf[ALL_SPI_DEVICES][10];

static struct spi_ioc_transfer _io_trans[2];

static int32 uart_fd;
const int8 *USB0_uart = "/dev/ttyUSB0";

const int8 *usb_uart = "/dev/ttymxc2";
const int8 *pump_uart = "/dev/ttymxc4";
#define BUFF_SIZE 256
#define BAUDRATE B115200
char readBuf[BUFF_SIZE];
unsigned char *readbuffer;

#ifdef _ONPC_
#define SETBIT(ADDRESS, BIT)
#define CLEARBIT(ADDRESS, BIT)
#define TESTBIT(ADDRESS, BIT)

#else
#define SETBIT(ADDRESS, BIT) (ADDRESS |= (1<<BIT))
#define CLEARBIT(ADDRESS, BIT) (ADDRESS &= ~(1<<BIT))
#define TESTBIT(ADDRESS, BIT) (ADDRESS & (1<<BIT))
#endif
#define IOMUXC_BASE_ADDR		0x020E0000



int i2c_speed=1;
int fd_SIC = -1;

int fd_volume;
int fd_RTC;
int fd_Temp_I2C;
int fd_FAN_I2C;
int fd_LED_I2C;
int fd_CQM_I2C;

bool check_write = false;
bool i2cdebug = true;
bool packetdebug = false;
unsigned char I2C_address;


//this i2c_related code sourced from below link!
//https://bytefreaks.net/tag/i2c_smbus_read_byte_data
static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
                                     int size, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args;
    int ret;
    //qDebug("smbus args:%x|%x|%x\n", read_write, static_cast<char>(command), size);
    args.read_write = read_write;
    args.command = command;
    args.size = size;
    args.data = data;

    ret = ioctl(file,I2C_SMBUS,&args);
    return ret;
}

static inline __s32 i2c_smbus_read_byte(int file)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data))
        return -1;
    else
        return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_read_bytes(int file, uint8* value, int size)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data))
        return NULL;
    else
    {
        memcpy(value, data.block, size);
        return 1;
    }
}


 static inline __s32 i2c_smbus_read_word_data(int file, __u8 command)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                         I2C_SMBUS_WORD_DATA,&data))
        return -1;
    else
        return 0x0FFFF & data.word;
}

static inline __s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
    union i2c_smbus_data data;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                         I2C_SMBUS_BYTE_DATA,&data))
        return -1;
    else
        return 0x0FF & data.byte;
}

static inline __s32 i2c_smbus_read_block_data(int file, __u8 command,
                                              __u8 *values)
{
    union i2c_smbus_data data;
    int i;
    if (i2c_smbus_access(file,I2C_SMBUS_READ,command,
                         I2C_SMBUS_BLOCK_DATA,&data))
        return -1;
    else {
        for (i = 1; i <= data.block[0]; i++)
            values[i-1] = data.block[i];
        return data.block[0];
    }
}

static inline __s32 i2c_smbus_write_i2c_block_data(int file, __u8 command,
                                                   __u8 length,
                                                   const __u8 *values)
{
    union i2c_smbus_data data;
    int i;
    if (length > 32)
        length = 32;
    for (i = 1; i <= length; i++)
        data.block[i] = values[i-1];
    data.block[0] = length;
    return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
                            I2C_SMBUS_I2C_BLOCK_BROKEN, &data);
}

static inline __s32 i2c_smbus_write_byte(int file, __u8 value)
{
    return i2c_smbus_access(file, I2C_SMBUS_WRITE, value,
                I2C_SMBUS_BYTE, NULL);
}

static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command, __u8 value)
{
    union i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_access(file,I2C_SMBUS_WRITE,command,
                            I2C_SMBUS_BYTE_DATA, &data);
}

static inline __s32 i2c_smbus_write_quick(int file, __u8 value)
{
    return i2c_smbus_access(file,value,0,I2C_SMBUS_QUICK,NULL);
}


//<------------------------------------------------------>

//<--------------------RTC & Volume----------------------->
//---------------------------------------------------------
//  Description:
//     Delay for at least 'len' ms
//



//---------------------------------------------------------------------------
//-------- Secured IC I2C Low-level functions
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// I2C Stop - Terminates the data transfer and releases the I2C bus
//



void I2C_stop(void)
{
   unsigned char sendpacket[2];

   // debug
   if (i2cdebug)
      ;//printf("P\n");
}

//---------------------------------------------------------------------------
// I2C Start - Issues a start condition and sends address and transfer
// direction
//
// Returns:  TRUE (1): start successful
//           FALSE (0): start failed
//
int I2C_start()
{
   // debug
   if (i2cdebug)
      ;//printf("S ");

   check_write = true;

   return true;
}

//---------------------------------------------------------------------------
// I2C Repeated start - Issues a repeated start condition and sends address
// and transfer direction
//
// Returns:  TRUE (1): repeated start successful
//           FALSE (0): repeated start failed
//
int I2C_rep_start()
{
   // debug
   if (i2cdebug)
      ;//printf("Sr ");

   check_write = true;

   return true;
}



//---------------------------------------------------------------------------
// I2C Set Speed
//
// 'speed':   1 fast mode
//            0 standard mode
//
// Returns:  TRUE (1): repeated start successful
//           FALSE (0): repeated start failed
//
int I2C_set_speed(int speed)
{

   // debug
   if (i2cdebug)
      ;//printf("<SP ");


   return true;
}

//---------------------------------------------------------------------------
//-------- I2C High-Level functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// I2C writeBlock
//
// 'i2c_addr'  : I2C addres
// 'addr'      : Memory address
// 'block_len' : write block length
// 'block'     : block buffer
//
// Returns:  TRUE (1): block written
//           FALSE (0): problem writing block
//
int I2C_writeBlock(int i2c_addr, int addr, int block_len, uchar *block)
{
   return 0;
}

//---------------------------------------------------------------------------
// I2C readBlock
//
// 'i2c_addr'  : I2C addres
// 'addr'      : Memory address
// 'block_len' : read block length
// 'block'     : block buffer
// 'skip_set_pointer : flag to indicate the device address pointer does not need to
//               be set before starting the read
//
// Returns:  TRUE (1): block read
//           FALSE (0): problem reading block
//
int I2C_readBlock(int i2c_addr, int addr, int block_len, uchar *block, int skip_set_pointer)
{
    return 0;
}

//---------------------------------------------------------------------------
// I2C writeReadBlock - Write and then read a block after a repeated start.
//
// 'i2c_addr'    : I2C addres
// 'addr'        : Memory address
// 'write_len'   : write block length
// 'write_block' : block buffer
// 'read_len'    : read block length
// 'read_block'  : block buffer
//
// Returns:  TRUE (1): block written and read
//           FALSE (0): problem reading block
//
int I2C_writeReadBlock(int i2c_addr, int addr, int write_len, uchar *write_block,
                       int read_len, uchar *read_block)
{
    return 0;
}

//---------------------------------------------------------------------------
// I2C readBlockPoll, write a block of data, do a repeated start, and then
// read waiting for a bit mask to match the 1's.
//
// 'i2c_addr'    : I2C addres
// 'write_len'   : write block length
// 'write_block' : block buffer
// 'ones_mask'   : bit mask indicating which bits must be 1's to stop polling
// 'zeros_mask'  : bit mask indicating which bits must be 0's to stop polling
// 'rslt'        : pointer to byte that contains the final value read when polling
//
// Returns:  TRUE (1): poll completed successfully
//           FALSE (0): poll on completed
//
int I2C_readBlockPoll(int i2c_addr, int write_len, uchar *write_block, uchar ones_mask, uchar zeros_mask, uchar *rslt)
{
   return 0;
}

//---------------------------------------------------------------------------
//-------- I2C Utility functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// I2C Connect - Setup I2C connection
//
int SecuredIC_Open(void)
{
    // debug
    if (i2cdebug)
        ;//printf("<Secured IC I2C connect>\n");

    fd_SIC = open("/dev/i2c-1", O_RDWR);

    if (fd_SIC < 0) {
        perror("Open error ");
        return false;
    }

    if (ioctl(fd_SIC, I2C_SLAVE, I2C_ADDR_SECUREDIC) < 0) {
        perror("ioctl error ln 24 ");
        return false;
    }

   return true;
}

int SecuredIC_Close(void)
{
    close(fd_SIC);
  //  printf("Close Volume /dev/i2c-1\n");
}
//---------------------------------------------------------------------------
// I2C Reset - Reset I2C adapter
//
void I2C_adapterReset(void)
{
   // debug
   if (i2cdebug)
      ;//printf("<I2C Adapter Reset>\n");
    //msDelay(N);

}



//---------------------------------------------------------------------------
// I2C Read Adapter Version
//
// 'version'  : pointer to byte where version will be saved
//
// Returns:  TRUE (1): version read
//           FALSE (0): failed to read version
//
int I2C_readAdapterVersion(uchar *version)
{
   unsigned char sendpacket[2];

   // debug
   if (i2cdebug)
      ;//printf("<I2C Adapter Read Version>\n");

    return 0;
}

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current
// global 'CRC8' value.
// Returns current global CRC8 value
//
static unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

static unsigned char docrc8(unsigned char value)
{
   // See Application Note 27
   CRC8 = dscrc_table[CRC8 ^ value];
   return CRC8;
}

void msDelay(int delay)
{
   int i;


   // special case while setting up adapter, long power up delay
   if (delay > 100)
   {
      for (i = 0; i < delay/100; i++)
      {
         mSleep(100);
         printf(".");
      }
   }
   else
      mSleep(delay);
}

int RTC_Open(void)
{
    fd_RTC = open("/dev/i2c-2", O_RDWR); //i2c-2 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_RTC < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }


    if (ioctl(fd_RTC, I2C_SLAVE, I2C_ADDR_MAX31343) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }
   // printf("open RTC /dev/i2c-2\n");

   // PT7C4339_Write();
   // printf("PT7C4339 Write_Success");


   // printf("\n");


}

int Volume_Open(void)
{
    fd_volume = open("/dev/i2c-0", O_RDWR); //i2c-0 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_volume < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }

    if (ioctl(fd_volume, I2C_SLAVE, I2C_ADDR_MAX9768) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }

   // printf("open Volume /dev/i2c-1\n");

   // MAX9767_Write();
   // printf("MAX9767 Write_Success");

  //  printf("\n");

}

int RTC_Close(void)
{
    close(fd_RTC);
   // printf("Close RTC /dev/i2c-2\n");
}

int Volume_Close(void)
{
    close(fd_volume);
  //  printf("Close Volume /dev/i2c-1\n");
}

//<--------------------Temp I2C----------------------->

int Temp_SHT40_I2C_Open(void)
{
    fd_Temp_I2C = open("/dev/i2c-2", O_RDWR); //i2c-0 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_Temp_I2C < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }

    if (ioctl(fd_Temp_I2C, I2C_SLAVE, I2C_ADDR_SHT40) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }

    //printf("open Temp_I2C /dev/i2c-2\n");

   // MAX9767_Write();
   // printf("MAX9767 Write_Success");

  //  printf("\n");

}
int Temp_SHT40_I2C_Close(void)
{
    close(fd_Temp_I2C);
  //  printf("Close Volume /dev/i2c-1\n");
}

int Temp_TMP117_I2C_Open(void)
{
    fd_Temp_I2C = open("/dev/i2c-2", O_RDWR); //i2c-0 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_Temp_I2C < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }

    if (ioctl(fd_Temp_I2C, I2C_SLAVE, I2C_ADDR_TMP117) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }

}

int Temp_TMP117_I2C_Close(void)
{
    close(fd_Temp_I2C);
}

int FAN_I2C_Open(void)
{
    fd_FAN_I2C = open("/dev/i2c-2", O_RDWR); //i2c-0 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_FAN_I2C < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }

    if (ioctl(fd_FAN_I2C, I2C_SLAVE, I2C_ADDR_MAX31760) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }


}
int FAN_I2C_Close(void)
{
    close(fd_FAN_I2C);
  //  printf("Close Volume /dev/i2c-1\n");
}


int LED_I2C_Open(void)
{
    fd_LED_I2C = open("/dev/i2c-2", O_RDWR); //i2c-0 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_LED_I2C < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }

    if (ioctl(fd_LED_I2C, I2C_SLAVE, I2C_ADDR_LP5009) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }

}
int LED_I2C_Close(void)
{
    close(fd_LED_I2C);
  //  printf("Close Volume /dev/i2c-1\n");
}

int CQM_I2C_Open(void)
{
    fd_CQM_I2C = open("/dev/i2c-1", O_RDWR); //i2c-0 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_CQM_I2C < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }

    if (ioctl(fd_CQM_I2C, I2C_SLAVE, I2C_ADDR_MAX1237) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }

}
int CQM_I2C_Close(void)
{
    close(fd_CQM_I2C);
  //  printf("Close Volume /dev/i2c-1\n");
}

int I2C_write(uchar data, int expect_ack)
{

    return true;
}



bool I2C_Write(I2C_State state, unsigned char *data, int length)
{
    /*
    if(i2cdebug)
    {
        for(int i=0; i<length; i++)
        {
            printf(">%02X ", *(data+i));
        }
        printf("\n");
    }
    */
    //printf("state = %d , data = %02X, length = %d",state, *data,length);
    switch(state)
    {
        case I2C_STATE_RTC:
                //printf("I2C_STATE_RTC");
                if (write(fd_RTC, data, length) != length)
                {
                    perror("RTC_write error");
                    return false;
                }
                break;
        case I2C_STATE_VOLUME:
                if (write(fd_volume, data, length) != length)
                {
                    perror("volume_write error");
                    return false;
                }
                break;
        case I2C_STATE_TEMP:
                //printf("I2C_STATE_TEMP");
                //printf("Temp I2C write = %d ", write(fd_Temp_I2C, data, length));
                if (write(fd_Temp_I2C, data, length) != length)
                {
                    perror("Temp_write error");
                    return false;
                }
                break;
        case I2C_STATE_FAN:
                //printf("I2C_STATE_FAN");
                if (write(fd_FAN_I2C, data, length) != length)
                {
                    perror("Fan_write error");
                    return false;
                }
                break;
        case I2C_STATE_SIC:
                //printf("I2C_STATE_SIC");
                if (write(fd_SIC, data, length) != length)
                {
                    perror("SIC_write error");
                    return false;
                }
                break;
        case I2C_STATE_LED:
                //printf("I2C_STATE_LED");
                if (write(fd_LED_I2C, data, length) != length)
                {
                    perror("LED_write error");
                    return false;
                }
                break;
        case I2C_STATE_CQM:
                //printf("I2C_STATE_LED");
                if (write(fd_CQM_I2C, data, length) != length)
                {
                    perror("CQM_write error");
                    return false;
                }
                break;

        default :
                break;

    }
    //printf("I2C_Write_Success");
    return true;

}


unsigned char* I2C_Read(I2C_State state, int ack, int *result)
{
    //unsigned char readbuffer[10];

    readbuffer = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * 10)) ;


    switch(state)
    {
        case I2C_STATE_RTC:
                if(read(fd_RTC, readbuffer, 1) != 1)
                {
                    perror("RTC_read error");
                    return 0;
                }
                //printf("readbuffer[0]= %02X \n", readbuffer[0]);
                break;
        case I2C_STATE_VOLUME:
                if(read(fd_volume, readbuffer, 1) != 1)
                {
                    perror("volume_read error");
                    return 0;
                }
                break;
        case I2C_STATE_TEMP:
                if(read(fd_Temp_I2C, readbuffer, 1) != 1)
                {
                    perror("Temp_read error");
                    return 0;
                }
                break;
        case I2C_STATE_FAN:
                if(read(fd_FAN_I2C, readbuffer, 1) != 1)
                {
                    perror("Fan_read error");
                    return 0;
                }
                break;

        default :
                break;

    }

    if (i2cdebug)
    {
        if (ack)
        {
            //printf("[%02X] ",readbuffer[0]);
        }
        else
        {
            //printf("[%02X*] ",readbuffer[0]);
        }
    }

    *result = true;

    if(state == I2C_STATE_TEMP)
    {
        return readbuffer;
        //return readbuffer[0];
    }
    else if(state == I2C_STATE_RTC)
    {
        return readbuffer;
        /*
        if (ioctl(fd_RTC, I2C_SLAVE, I2C_ADDR_MAX31343) < 0) {
            printf("ioctl error ln 24 : %s\n", strerror(errno));

        }
        return readbuffer;
        */
    }
    else
    {

    }

    // failure to read expected byte, reset adapter
    I2C_adapterReset();
    *result = false;

    //return false;
}

//---------------------------------------------------------------------------
// I2C Read - read one byte from the I2C device
//
// Parameters:  ack - (1) send ACK, request more data from device
//                    (0) send NAK, read is followed by a stop condition
//              result - pointer to integer containing the operation result
//                       (0) to indicate a failure (1) to indication success
//
// Returns:  byte read from I2C device
//
uchar I2C_read(int ack, int *result) //SIC I2C_read
{
    unsigned char readbuffer[10];

    if(read(fd_SIC, readbuffer, 1) != 1){
        perror("read error");
        return 0;
    }
    else
    {
        if (i2cdebug)
        {
            if (ack)
                ;//printf("[%02X] ",readbuffer[0]);
            else
                ;//printf("[%02X*] ",readbuffer[0]);
        }

        *result = true;
        return readbuffer[0];
    }

    // failure to read expected byte, reset adapter
    I2C_adapterReset();
    *result = false;

    return false;
}



int MAX9768_Write(uint32 OM, uint32 DataValue) //OM : OutputModulation
{
    int result;
    unsigned char sendbuffer[10], readbuffer[10];
    int cnt = 0, poll_count=0;
    unsigned char status;
    i2c_smbus_data smbusData;
    smbusData.byte= (uint8)OM;

#if (USING_SMBUS == true)
    //result = i2c_smbus_access(fd_volume,0,0,I2C_SMBUS_QUICK, &smbusData);
    result = i2c_smbus_write_byte(fd_volume, (unsigned char)OM);
    if(result){
        qDebug("OM]err to set vol code :%d", result);
    }

    result = i2c_smbus_write_byte(fd_volume, (unsigned char)DataValue);
    if(result)
    {
        qDebug("data]err to set vol code : %d", result);
    }

    return result;
#else
    sendbuffer[cnt++] = OM; // OutputModulation
    if(!I2C_Write(I2C_STATE_VOLUME, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    I2C_Read(I2C_STATE_VOLUME, ACK, &result);

    cnt=0;
    sendbuffer[cnt++] = DataValue; //Volume Data Value (0x00~0x3f)
    if(!I2C_Write(I2C_STATE_VOLUME, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    I2C_Read(I2C_STATE_VOLUME, ACK, &result);
#endif
}

int MAX31343_Write(uint32 Registers , uint32 DataValue) //RTC
{
    unsigned char sendbuffer[10];
    int cnt = 0;
    fd_RTC = open("/dev/i2c-2", O_RDWR); //i2c-2 : i2c bus number
                                     //i2c Checking : i2cdetect -y 0~2
    if (fd_RTC < 0) {
        printf("Open error: %s\n", strerror(errno));
        return 1;
    }
    if (ioctl(fd_RTC, I2C_SLAVE, I2C_ADDR_MAX31343) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }
    sendbuffer[cnt++] = Registers; //Data Register 0x05
    sendbuffer[cnt++] = DataValue; //RTC Data Value 0x10 (0x00~0xff)
    if(!I2C_Write(I2C_STATE_RTC, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    close(fd_RTC);
}

unsigned char MAX31343_Read(uint32 REG_ADDR)
{
    struct i2c_rdwr_ioctl_data data;
    data.msgs = (struct i2c_msg *)malloc(2 * sizeof(struct i2c_msg));
    unsigned char sendbuf[sizeof(unsigned char) + 1] = {0};
    unsigned char recvbuf[sizeof(unsigned char) + 1] = {0};
    uint32 I2C_ADDR = I2C_ADDR_MAX31343;
    fd_RTC = open("/dev/i2c-2", O_RDWR); //i2c-2 : i2c bus number
                                    //i2c Checking : i2cdetect -y 0~2
    if (fd_RTC < 0) {
       printf("Open error: %s\n", strerror(errno));
       return 1;
    }
    qDebug("Start MAX31343_Read");

    //2.read reg value
    data.nmsgs = 2;
    data.msgs[0].len = 1;//  reg
    data.msgs[0].addr = I2C_ADDR; //i2c
    data.msgs[0].flags = 0; //write flag
    data.msgs[0].buf = sendbuf;//     2 Byte
    data.msgs[0].buf[0] = REG_ADDR;//  reg

    data.msgs[1].len = 1;//  reg
    data.msgs[1].addr = I2C_ADDR; //i2c
    data.msgs[1].flags = 1; //read flag
    data.msgs[1].buf = recvbuf;//

    ioctl(fd_RTC,I2C_RDWR,(unsigned long)&data);
    qDebug("buf[0] = %x",data.msgs[1].buf[0]);

    close(fd_RTC);
    return data.msgs[1].buf[0];
}

int MAX1237_Write(uint32 WriteData)
{
    int result;
    unsigned char sendbuffer[10], readbuffer[10];
    int cnt = 0, poll_count=0;
    unsigned char status;

    cnt=0;
    fd_CQM_I2C = open("/dev/i2c-1", O_RDWR); //i2c-0 : i2c bus number
                                    //i2c Checking : i2cdetect -y 0~2
    if (fd_CQM_I2C < 0) {
       printf("Open error: %s\n", strerror(errno));
       return 1;
    }
    if (ioctl(fd_CQM_I2C, I2C_SLAVE, I2C_ADDR_MAX1237) < 0) {
        printf("ioctl error ln 24 : %s\n", strerror(errno));
        return 1;
    }


    sendbuffer[cnt++] = WriteData; //SetupByte or ConfigurationByte Setting
    if(!I2C_Write(I2C_STATE_CQM, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    close(fd_CQM_I2C);

}
unsigned short MAX1237_Read()
{

    struct i2c_rdwr_ioctl_data data;
    data.msgs = (struct i2c_msg *)malloc(2 * sizeof(struct i2c_msg));
    unsigned char sendbuf[sizeof(unsigned char) + 1] = {0};
    unsigned char recvbuf[2] = {0,};
    unsigned short ret = -1;
    uint32 I2C_ADDR = I2C_ADDR_MAX1237;
    fd_CQM_I2C = open("/dev/i2c-1", O_RDWR); //i2c-0 : i2c bus number
                                    //i2c Checking : i2cdetect -y 0~2
    if (fd_CQM_I2C < 0) {
       printf("Open error: %s\n", strerror(errno));
       return 1;
    }

    //2.read reg value
    data.nmsgs = 1;
    data.msgs[0].len = 2;//  reg
    data.msgs[0].addr = I2C_ADDR; //i2c
    data.msgs[0].flags = 1; //read flag
    data.msgs[0].buf = recvbuf;//

    ioctl(fd_CQM_I2C,I2C_RDWR,(unsigned long)&data);

    close(fd_CQM_I2C);
    ((unsigned char*)&ret)[1] = data.msgs[0].buf[0] & 0x0F;
    ((unsigned char*)&ret)[0] = data.msgs[0].buf[1];
    //qDebug("ret = %d", ret);
    return ret;

}



int SHT40_Write(uint32 Address , uint32 Command ) //Temp I2C
{
    int result=0;
    unsigned char sendbuffer[10];
    unsigned char *rx_bytes;//
    char *tmp;
    int cnt = 0;
    uint32 t_ticks = 0, rh_ticks = 0 , t_degC = 0 , rh_pRH = 0;

    rx_bytes = (unsigned char*)malloc(10);

    i2c_smbus_write_byte(fd_Temp_I2C, Command);

    //msDelay(1000);
    mdelay(10);
    memset(rx_bytes, 0x00, 10);
    /*
    rx_bytes = I2C_Read(I2C_STATE_TEMP, ACK, &result);

    for(int n= 0 ; n < 10 ; n++)
    {
        printf("rx_bytes [%d] = %d \n", n ,rx_bytes[n]);
    }
    */

    result = i2c_smbus_read_bytes(fd_Temp_I2C, rx_bytes, 10);
    /*
    for(int n= 0 ; n < 10 ; n++)
    {
        printf("smbus] rx_bytes [%d] = %d \n", n ,rx_bytes[n]);
    }
    */
    t_ticks = rx_bytes[0] * 256 + rx_bytes[1];
    rh_ticks = rx_bytes[3] * 256 + rx_bytes[4];
    //printf("t_ticks = %d\n",t_ticks);
    //printf("rh_ticks = %d\n",rh_ticks);

    t_degC = -45 + 175 * t_ticks/65535;
    rh_pRH = -6 + 125 * rh_ticks/65535;
    if (rh_pRH > 100)
        rh_pRH = 100;
    if (rh_pRH < 0)
        rh_pRH = 0;
    //printf("t_degC = %d\n", t_degC);
    //printf("rh_pRH = %d\n", rh_pRH);

   free(rx_bytes);
   return t_degC;
}


uint32 SHT40_Read(uint32 Registers) //Temp I2C
{
    qDebug("%s", __FUNCTION__);
    int result;
    unsigned char sendbuffer[10], readbuffer[10];
    int cnt = 0, poll_count=0;
    unsigned char status;

#if (USING_SMBUS == true)
    int data=0;
    //data = i2c_smbus_read_byte_data(fd_Temp_I2C, Registers);
    data = i2c_smbus_read_byte(fd_Temp_I2C);
    qDebug("data : %d", data);
    return static_cast<char>(data);
#else
    sendbuffer[cnt++] = Registers; //RTC Data Value
    if(!I2C_Write(I2C_STATE_RTC, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    I2C_Read(I2C_STATE_RTC, ACK,&result);
#endif
}

int MAX31760_Write(uint32 Registers , uint32 DataValue)
{
    int result;
    unsigned char sendbuffer[10], readbuffer[10];
    int cnt = 0, poll_count=0;
    unsigned char status;

#if (USING_SMBUS == true)
    result = i2c_smbus_write_byte_data(fd_FAN_I2C, Registers, (unsigned char)DataValue);
    return result;
#else
/*
    sendbuffer[cnt++] = 0x00; //Write bit Data Value
    if(!I2C_Write(sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    I2C_Read(ACK,&result);
*/
    cnt=0;
    sendbuffer[cnt++] = Registers; //Data Register 0x05
    sendbuffer[cnt++] = DataValue; //RTC Data Value 0x10 (0x00~0xff)
    if(!I2C_Write(I2C_STATE_FAN, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    I2C_Read(I2C_STATE_FAN, ACK, &result);
#endif
}

unsigned char MAX31760_Read(uint32 Registers)
{
    int result;
    unsigned char sendbuffer[10];
    unsigned char *readbuf;
    int cnt = 0, poll_count=0;
    unsigned char status;

#if (USING_SMBUS == true)
    int data;
    data = i2c_smbus_read_byte_data(fd_FAN_I2C, Registers);
    return static_cast<char>(data);
#else
    sendbuffer[cnt++] = Registers; //RTC Data Value
    if(!I2C_Write(I2C_STATE_FAN, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    readbuf = I2C_Read(I2C_STATE_FAN, ACK,&result);
    return readbuf[0];
#endif
}

unsigned char* TMP117_Read(uint32 Registers) //Temp TMP117 I2C
{
    int result;
    unsigned char sendbuffer[10];
    unsigned char* readbuf;
    int cnt = 0, poll_count=0;
    unsigned char status;

    sendbuffer[cnt++] = Registers; //RTC Data Value
    if(!I2C_Write(I2C_STATE_TEMP, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    readbuf = I2C_Read(I2C_STATE_TEMP, ACK,&result);
    return readbuf;

}

int LP5009_Write(uint32 Registers , uint32 DataValue)
{
    int result;
    unsigned char sendbuffer[10], readbuffer[10];
    int cnt = 0, poll_count=0;
    unsigned char status;

    cnt=0;
    sendbuffer[cnt++] = Registers; //Data Register
    sendbuffer[cnt++] = DataValue; //Data Value
    if(!I2C_Write(I2C_STATE_LED, sendbuffer, cnt))
    {
        printf("send error\n");
        return 0;
    }
    I2C_Read(I2C_STATE_LED, ACK, &result);

}


volatile unsigned *mapRegAddr(unsigned long baseAddr)
{
    void *regAddrMap = MAP_FAILED;
    int mem_fd = 0;

    if(!mem_fd)
    {
        if((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
        {
            cout << "Cant open /dev/mem" << endl;
            exit(1);
        }
    }

    regAddrMap = mmap(NULL, 32*8,
                      PROT_READ|PROT_WRITE|PROT_EXEC,
                      MAP_SHARED, mem_fd, baseAddr);

    if(regAddrMap == MAP_FAILED)
    {
        cout << "mmap is failed" << endl;
        exit(1);
    }


    if(close(mem_fd) < 0)
    {
        cout << "cant close /dev/mem" << endl;
        exit(1);

    }
    return (volatile unsigned *)regAddrMap;

}
#ifndef _ONPC_
int32 uart_init()
{

    //unsigned long baudrate = 115200;
    struct termios options;
    char *tx = "USB Open\n";

    //uart_fd = open("/dev/ttymxc1", O_RDWR | O_NOCTTY);

    //uart_fd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY);
    uart_fd = open(usb_uart , O_RDWR | O_NOCTTY);
    fcntl(uart_fd, F_SETFL, 0);

    tcgetattr(uart_fd, &options);
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;

    options.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT );

    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    tcsetattr(uart_fd, TCSANOW, &options);
    write(uart_fd, tx, strlen(tx));
    return uart_fd;
}

void uart_close()
{
    char *tx = "USB Close\n";
    write(uart_fd, tx, strlen(tx));
    close(uart_fd);
}

void uart_binwrite(unsigned char *data, int length)
{
    write(uart_fd, data, length);
}

char* uart_binread(int *length)
{
    *length = read(uart_fd, readBuf, BUFF_SIZE);
    //readBuf[*length] = '\0';
    return readBuf;
}

void uart_printf(char *pcFmt,...)
{
    va_list ap;
    char pbString[256];
    va_start(ap,pcFmt);
    vsprintf(pbString,pcFmt,ap);
    write(uart_fd, pbString, strlen(pbString));
    va_end(ap);
}
#endif

void gpio_init()
{
#ifndef _ONPC_
    gpio1_address = mapRegAddr(GPIO1_DR);
    gpio2_address = mapRegAddr(GPIO2_DR);
    gpio3_address = mapRegAddr(GPIO3_DR);
    gpio4_address = mapRegAddr(GPIO4_DR);
    gpio5_address = mapRegAddr(GPIO5_DR);
    gpio6_address = mapRegAddr(GPIO6_DR);
#endif
    /*
    int gpio_fd = open("/sys/class/gpio/export", O_WRONLY);
    if (gpio_fd == -1) {
        perror("Unable to open /sys/class/gpio/export");
        exit(1);
    }

    if (write(gpio_fd, "95", 2) != 2) {
        perror("Error writing to /sys/class/gpio/export");
        exit(1);
    }
    */
    //WriteGPIOPortLow(GPIO_3, 20);
}

volatile unsigned *GetEncorderAddress()
{
    return gpio2_address;
}

uint32 ReadGPIOPort(uint32 gpio, uint32 pin)
{

Q_UNUSED(gpio);
Q_UNUSED(pin);
#ifdef _ONPC_
    uint32 result = 1;
    return result;
#else

    switch(gpio)
    {
        case GPIO_1:
            if(TESTBIT(*(gpio1_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_2:
            if(TESTBIT(*(gpio2_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_3:
            if(TESTBIT(*(gpio3_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_4:
            if(TESTBIT(*(gpio4_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_5:
            if(TESTBIT(*(gpio5_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_6:
            if(TESTBIT(*(gpio6_address), pin)) return GPIO_HIGH;
            break;
    }

    return GPIO_LOW;
#endif
}


void WriteGPIOPort(uint32 gpio, uint32 pin, bool flag)
{
    /*
     * 	flag : true is HI, false is LOW
     *
     */
Q_UNUSED(gpio);
Q_UNUSED(pin);
Q_UNUSED(flag);
#ifdef _ONPC_
    //qDebug("gpio=%d, pin=%d, flag=%d\n", gpio, pin, flag);
#else
    switch(gpio)
    {
        case GPIO_1:
            if(flag)
                SETBIT(*(gpio1_address), pin);    //HI
            else
                CLEARBIT(*(gpio1_address), pin);    //Low
            break;

        case GPIO_2:
            if(flag)
                SETBIT(*(gpio2_address), pin);    //HI
            else
                CLEARBIT(*(gpio2_address), pin);    //Low
            break;

        case GPIO_3:
            if(flag)
                SETBIT(*(gpio3_address), pin);    //HI
            else
                CLEARBIT(*(gpio3_address), pin);    //Low
            break;
        case GPIO_4:
            if(flag)
                SETBIT(*(gpio4_address), pin);    //HI
            else
                CLEARBIT(*(gpio4_address), pin);    //Low
            break;

        case GPIO_5:
            if(flag)
                SETBIT(*(gpio5_address), pin);    //HI
            else
                CLEARBIT(*(gpio5_address), pin);    //Low
            break;

        case GPIO_6:
            if(flag)
                SETBIT(*(gpio6_address), pin);    //HI
            else
                CLEARBIT(*(gpio6_address), pin);    //Low
            break;

        default:
            break;
    }
#endif
}

void WriteGPIOPortHigh(uint32_t port_id, uint32_t port_value)
{
    WriteGPIOPort(port_id, port_value, GPIO_HIGH);
}

void WriteGPIOPortLow(uint32_t port_id, uint32_t port_value)
{
    WriteGPIOPort(port_id, port_value, GPIO_LOW);
}
void LCDBackLight_ON()
{
    WriteGPIOPort(GPIO_2 , 5 , GPIO_HIGH);
}
void LCDBackLight_OFF()
{
    WriteGPIOPort(GPIO_2 , 5 , GPIO_LOW);
}

void AudioAMPControl_Enable()
{
    qDebug() << "AudioAMPControl_Enable" ;
    //WriteGPIOPort(GPIO_2, 1, GPIO_HIGH); //Audio AMP_Control ON
    WriteGPIOPortHigh(GPIO_2, 1);

}
void AudioAMPControl_Disable()
{
    qDebug() << "AudioAMPControl_Disable" ;
    //WriteGPIOPort(GPIO_2, 1, GPIO_LOW); //Audio AMP_Control OFF
    WriteGPIOPortLow(GPIO_2, 1);
}

void EnableAmplifierPowerOutput()
{
    //printf("EnableAmp High");
    qDebug() << "EnableAmp High";

    //WriteGPIOPort(GPIO_5, 8, GPIO_LOW);   //DAC Buffer ON
    //WriteGPIOPort(GPIO_3, 31, GPIO_HIGH);   //Target Relay
    //WriteGPIOPort(GPIO_5, 9, GPIO_LOW);   //Target Relay Buffer ON
    WriteGPIOPortHigh(GPIO_3, 29);   //Isolated DCDC Converter Remote Control
    msDelay(400);
    WriteGPIOPortHigh(GPIO_3, 31);   //Target Relay    
    WriteGPIOPortHigh(GPIO_4, 15);   //Target Relay
    msDelay(20);
}

void DisableAmplifierPowerOutput()
{
    qDebug() << "DisableAmp Low";
    //printf("DisableAmp Low");

    //WriteGPIOPort(GPIO_5, 8, GPIO_HIGH);   //DAC Buffer OFF
    //WriteGPIOPort(GPIO_3, 31, GPIO_LOW);   //Target Relay+
    msDelay(20);
    WriteGPIOPortLow(GPIO_3, 29);   //Isolated DCDC Converter Remote Control
    msDelay(20);
    WriteGPIOPortLow(GPIO_4, 15);   //Target Relay
    msDelay(20); //20
    WriteGPIOPortLow(GPIO_3, 31);   //Target Relay
}

void EnableIsolated_DCDC()
{
    //printf("EnableAmp High");
    qDebug() << "EnableDCDC High";

    //WriteGPIOPort(GPIO_5, 8, GPIO_LOW);   //DAC Buffer ON
    //WriteGPIOPort(GPIO_3, 31, GPIO_HIGH);   //Target Relay
    //WriteGPIOPort(GPIO_5, 9, GPIO_LOW);   //Target Relay Buffer ON
    WriteGPIOPortHigh(GPIO_3, 29);   //Isolated DCDC Converter Remote Control
    //WriteGPIOPortHigh(GPIO_3, 31);   //Target Relay
}
void DisableIsolated_DCDC()
{
    //printf("EnableAmp High");
    qDebug() << "DisableDCDC High";
    //WriteGPIOPort(GPIO_5, 8, GPIO_LOW);   //DAC Buffer ON
    //WriteGPIOPort(GPIO_3, 31, GPIO_HIGH);   //Target Relay
    //WriteGPIOPort(GPIO_5, 9, GPIO_LOW);   //Target Relay Buffer ON
    WriteGPIOPortLow(GPIO_3, 29);   //Isolated DCDC Converter Remote Control
    //WriteGPIOPortHigh(GPIO_3, 31);   //Target Relay
}

void EnableTargetRelay_1st()
{
    qDebug() << "EnableTarget 1st Relay High";
    WriteGPIOPortHigh(GPIO_4, 15);   //1st_Target Relay
}
void EnableTargetRelay_2nd()
{
    qDebug() << "EnableTarget 2nd Relay High";
    WriteGPIOPortHigh(GPIO_3, 31);   //2nd_Target Relay
}

void DisableTargetRelay_1st()
{
    qDebug() << "DisableTarget 1st Relay Low";
    WriteGPIOPortLow(GPIO_4, 15);   //Target Relay
}
void DisableTargetRelay_2nd()
{
    qDebug() << "DisableTarget 2nd Relay Low";
    WriteGPIOPortLow(GPIO_3, 31);   //Target Relay
}
void RFSwitchLED_ON()
{
    //qDebug() << "RFSwitchLED ON";
    WriteGPIOPortHigh(GPIO_1, 17);
}
void RFSwitchLED_OFF()
{
    //qDebug() << "RFSwitchLED OFF";
    WriteGPIOPortLow(GPIO_1, 17);
}

void SetElectrodeType(ELECTRODE_Type_en eType)
{
    switch(eType)
    {
        case ELECTRODE_MONOPOLAR:
            WriteGPIOPort(GPIO_3, 31, GPIO_HIGH);
            WriteGPIOPort(GPIO_3, 30, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 29, GPIO_LOW);
            break;

        case ELECTRODE_BI_3CM:
            WriteGPIOPort(GPIO_3, 31, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 30, GPIO_HIGH);
            WriteGPIOPort(GPIO_3, 29, GPIO_LOW);
            break;

        case ELECTRODE_BI_7CM:
            WriteGPIOPort(GPIO_3, 31, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 30, GPIO_HIGH);
            WriteGPIOPort(GPIO_3, 29, GPIO_HIGH);
            break;

        default:
            WriteGPIOPort(GPIO_3, 31, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 30, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 29, GPIO_LOW);
            break;
    }
}

void SetAmplifierPowerOutputDirection(RF_OutputDir_en dir)
{
    if(dir == RFOUTPUT_DIR_TARGET)
    {
        WriteGPIOPort(GPIO_2, 18, GPIO_HIGH);    //RF_IN_DIR
    }
    else
    {
        WriteGPIOPort(GPIO_2, 18, GPIO_LOW);
    }
}
void FAN_RPM_Read()
{
    unsigned char TC1H,TC1L;
    uint32 FAN_RPM;
    FAN_I2C_Open();
    TC1H = MAX31760_Read(FAN_REGISTERS_TC1H);
    TC1L = MAX31760_Read(FAN_REGISTERS_TC1L);
    //qDebug("TC1H = %02X , TC1L = %02X", TC1H , TC1L);

    //FAN_RPM = 60*100000/(TC1H*256 + TC1L) /2;

    //Fan_RPM = ( 6000000 / (((TACH1_MSB * 256) + TACH1_LSB)))//2
    FAN_RPM = ( 3000000 / (((TC1H * 256) + TC1L)));
    FAN_I2C_Close();
    //qDebug("FAN_RPM = %d" , FAN_RPM);
}

unsigned char FAN_Temp_Read()
{
    unsigned char RTH,RTL;

    FAN_I2C_Open();
    RTH = MAX31760_Read(FAN_REGISTERS_RTH);
    RTL = MAX31760_Read(FAN_REGISTERS_RTL);

    //qDebug("FAN_RTH = %02X , FAN_RTL = %02X", RTH , RTL);
    //msDelay(1000);
    FAN_I2C_Close();
    //sysParm->mRfDeviceFactors->fan_temp_value = RTH;
    //qDebug("FAN_Temp = %d", FAN_Temp);
    return RTH;
}

void FAN_Setting()
{
#ifndef _ONPC_
    qDebug() << "FAN_Setting ";
    //printf("FAN_Setting ");


    MAX31760_Write(FAN_REGISTERS_CR1, 0x19); //DRV = 25kHz. Remote temperature
    MAX31760_Write(FAN_REGISTERS_CR2, 0x01); //Direct fan control enable
    MAX31760_Write(FAN_REGISTERS_CR3, 0xB1); //CLR_FAIL = clear , RAMP0~1 = Fast , TACH1E = enable
    MAX31760_Write(FAN_REGISTERS_TCTH, 0xFF);
    MAX31760_Write(FAN_REGISTERS_TCTL, 0xFE);
    MAX31760_Write(FAN_REGISTERS_FFDC, 0xFF);
    MAX31760_Write(FAN_REGISTERS_PWMR, 0x7F); //50%


    //gpioTools::msDelay(3000);
    //FAN_I2C_Close();
#endif
}
void FAN_Write(unsigned int fan_temp_value )
{
    uint32 FAN_DTC;
    //FAN_DTC = FAN_Temp*2;
    //FAN_DTC  = FAN_Temp // 24*2 = 48%

    FAN_DTC = 256 * (fan_temp_value *2) /100;
    if(FAN_DTC >=256)
    {
        FAN_DTC = 255;
    }
    FAN_I2C_Open();

    if((fan_temp_value *2) <= 100)
    {
        MAX31760_Write(FAN_REGISTERS_PWMR, FAN_DTC);
    }

    FAN_I2C_Close();
        //qDebug("FAN_DTC_HEX = %02X , FAN_DTC_DEC = %d", FAN_DTC, FAN_DTC);


}


void PWM_RGB(int RGB_Color, int dutycycle_count)
{

    //qDebug()<< "PWM_OPEN";
#ifndef _ONPC_
    FILE *loadOverlay = NULL;
    FILE *pwm1_RED = NULL;
    FILE *pwm2_BLUE = NULL;
    FILE *pwm4_GREEN = NULL;
    FILE *period_file = NULL;
    FILE *dutyCycle_file = NULL;
    FILE *enable_file = NULL;
    FILE *disable_file = NULL;


    char overlay[] = "BB-PWM1";
    char pwm0[] = "0";

    char period[] = "1000"; //"100000000"
    char dutyCycle[10]; //50 //"5000000"

    char enable[] = "1";
    char disable[] = "0";
    int pwm_max;
    int pwm_min=0;

    //QString dutycyle_str;

    /* Load the DTO to enable PWM */
    /*
    loadOverlay = fopen("/sys/devices/platform/bone_capemgr/slots", "w");
    fwrite(overlay,1,sizeof(overlay), loadOverlay);
    fclose(loadOverlay);
    */
    pwm1_RED = fopen("/sys/class/pwm/pwmchip0/export", "w");
    fwrite(pwm0,1,sizeof(pwm0),pwm1_RED);
    fclose(pwm1_RED);
    pwm4_GREEN = fopen("/sys/class/pwm/pwmchip3/export", "w");
    fwrite(pwm0,1,sizeof(pwm0),pwm4_GREEN);
    fclose(pwm4_GREEN);
    pwm2_BLUE = fopen("/sys/class/pwm/pwmchip1/export", "w");
    fwrite(pwm0,1,sizeof(pwm0),pwm2_BLUE);
    fclose(pwm2_BLUE);

    /*We are using pins P9_14 and and P9_16 pins for this PWM */
    switch(RGB_Color)
    {
        case 0: //RED


            period_file = fopen("/sys/class/pwm/pwmchip0/pwm0/period", "w");
            fwrite(period,1,sizeof(period),period_file);
            //fwrite(period,1,sizeof(period),period_file);

            fclose(period_file);

            //qDebug() << "dutycycle_count = " << dutycycle_count;
            sprintf(dutyCycle, "%d" , dutycycle_count);
            dutyCycle_file = fopen("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", "w");
            //fwrite(period,1,dutycycle_count,dutyCycle_file);
            fwrite(dutyCycle,1,sizeof(dutyCycle),dutyCycle_file);
            fclose(dutyCycle_file);


            enable_file = fopen("/sys/class/pwm/pwmchip0/pwm0/enable", "w");
            fwrite(enable,1,sizeof(enable),enable_file);
            fclose(enable_file);
            disable_file = fopen("/sys/class/pwm/pwmchip1/pwm0/enable", "w");
            fwrite(disable,1,sizeof(disable),disable_file);
            fclose(disable_file);

            disable_file = fopen("/sys/class/pwm/pwmchip3/pwm0/enable", "w");
            fwrite(disable,1,sizeof(disable),disable_file);
            fclose(disable_file);

            //qDebug() << "RGB_RED" ;
            break;
        case 1: //GREEN


            period_file = fopen("/sys/class/pwm/pwmchip3/pwm0/period", "w");
            fwrite(period,1,sizeof(period),period_file);
            //fwrite(period,1,sizeof(period),period_file);

            fclose(period_file);
            //qDebug() << "dutycycle_count = " << dutycycle_count;
            sprintf(dutyCycle, "%d" , dutycycle_count);
            dutyCycle_file = fopen("/sys/class/pwm/pwmchip3/pwm0/duty_cycle", "w");
            //fwrite(period,1,dutycycle_count,dutyCycle_file);
            fwrite(dutyCycle,1,sizeof(dutyCycle),dutyCycle_file);
            fclose(dutyCycle_file);


            disable_file = fopen("/sys/class/pwm/pwmchip0/pwm0/enable", "w");
            fwrite(disable,1,sizeof(disable),disable_file);
            fclose(disable_file);

            disable_file = fopen("/sys/class/pwm/pwmchip1/pwm0/enable", "w");
            fwrite(disable,1,sizeof(disable),disable_file);
            fclose(disable_file);

            enable_file = fopen("/sys/class/pwm/pwmchip3/pwm0/enable", "w");
            fwrite(enable,1,sizeof(enable),enable_file);
            fclose(enable_file);

            //qDebug() << "RGB_GREEN" ;
            break;
        case 2: //BLUE


            period_file = fopen("/sys/class/pwm/pwmchip1/pwm0/period", "w");
            fwrite(period,1,sizeof(period),period_file);
            //fwrite(period,1,sizeof(period),period_file);

            fclose(period_file);
            //qDebug() << "dutycycle_count = " << dutycycle_count;
            sprintf(dutyCycle, "%d" , dutycycle_count);

            dutyCycle_file = fopen("/sys/class/pwm/pwmchip1/pwm0/duty_cycle", "w");
            //fwrite(period,1,dutycycle_count,dutyCycle_file);
            fwrite(dutyCycle,1,sizeof(dutyCycle),dutyCycle_file);
            fclose(dutyCycle_file);


            disable_file = fopen("/sys/class/pwm/pwmchip0/pwm0/enable", "w");
            fwrite(disable,1,sizeof(disable),disable_file);
            fclose(disable_file);

            enable_file = fopen("/sys/class/pwm/pwmchip1/pwm0/enable", "w");
            fwrite(enable,1,sizeof(enable),enable_file);
            fclose(enable_file);

            disable_file = fopen("/sys/class/pwm/pwmchip3/pwm0/enable", "w");
            fwrite(disable,1,sizeof(disable),disable_file);
            fclose(disable_file);
            //qDebug() << "RGB_BLUE" ;
            break;

        default :
            break;
    }


    /*
    dutyCycle_file = fopen("/sys/class/pwm/pwmchip1/pwm0/duty_cycle", "w");
    fwrite(period,1,sizeof(dutyCycle),dutyCycle_file);
    fclose(dutyCycle_file);
    */
    //qDebug() << "dutycycle_count = " << dutycycle_count;
    //sprintf(dutyCycle, "%d" , dutycycle_count);

    //strcpy(dutyCycle, );
    //dutycyle_str.sprintf("%d", i);





    sscanf(period, "%d", &pwm_max);
    //qDebug() << "period_dec = " << pwm_max;
    //qDebug() << "dutycycle_count = " <<dutycycle_count;
    /*
    if(dutycycle_count < pwm_max)
    {
       dutycycle_count += 10;
       PWM_State = true;
    }
    else
    {
       PWM_State = false;
       dutycycle_count -= 10;
    }
    */

    //usleep(10000000);

    /*
    enable_file = fopen("/sys/class/pwm/pwmchip0/pwm0/enable", "w");
    fwrite(disable,1,sizeof(disable),enable_file);
    fclose(enable_file);
    */

    //qDebug() << "PWM_RGB_Test_Success" ;
#endif

}

void OnLED(LED_Output_en id)
{
    switch(id)
    {
        case Leds_HANDLE_LED:
            WriteGPIOPort(GPIO_2, 28, GPIO_HIGH);
            break;
  }
}

void OffLED(LED_Output_en id)
{
    switch(id)
    {
        case Leds_HANDLE_LED:
            WriteGPIOPort(GPIO_2, 28, GPIO_LOW);
            break;

    }

}

  void StopSound()
  {
      std::cout << "StopSound" << std::endl;
  }

  void StartSound(uint32_t id)
  {
      Q_UNUSED(id);
      std::cout << "StartSound" << std::endl;
  }


void spi_init()
{
    qDebug() << "spi_init";
    for(int32 i=0; i<ALL_SPI_DEVICES; i++)
    {
        _spifd[i] = -1;
    }


    spiOpen(SPI_DAC8830);
    spiOpen(SPI_ADS8343_A);
    spiOpen(SPI_ADS8343_T);

}

void spiAllclose()
{
    for(int32 i = 0; i<ALL_SPI_DEVICES; i++)
    {
        if(_spifd[i] != -1)
        {
            spiClose(_spifd[i]);
        }
        _spifd[i] = -1;
    }
}



int32 spiOpen(SPI_DEVICES dn)
{
    std::string devspi;

    /*
     *  SPI_CPOL = 0 : low clock ,
     * 							1 : High clock
     *
     * SPI_CPHA = 0 : rising edge
     *                   			1 : falling edge
     *
     *  SPI_MODE_0		(0|0)
     *	 SPI_MODE_1		(0|SPI_CPHA)
     *	 SPI_MODE_2		(SPI_CPOL|0)
     *	 SPI_MODE_3		(SPI_CPOL|SPI_CPHA)
     *
     */

    switch(dn)
    {
        case SPI_DAC8830:
             devspi = "/dev/spidev0.0";
             _mode[SPI_DAC8830] = SPI_MODE_3;
             _bits_per_word[SPI_DAC8830] = 16;
             _speed[SPI_DAC8830] = 2000000; //50M
             break;

        case SPI_ADS8343_A:
            devspi = "/dev/spidev1.0";
            _mode[SPI_ADS8343_A] = SPI_MODE_0;
            _bits_per_word[SPI_ADS8343_A] = 8;
            _speed[SPI_ADS8343_A] = 2000000;
            break;

        case SPI_ADS8343_T:
            devspi = "/dev/spidev1.1";
            _mode[SPI_ADS8343_T] = SPI_MODE_0;
            _bits_per_word[SPI_ADS8343_T] = 8;
            _speed[SPI_ADS8343_T] = 2000000;
            break;

        case SPI_VOL_CTRL_1:
            devspi = "/dev/spidev2.0";
            _mode[SPI_VOL_CTRL_1] = SPI_MODE_0;
            _bits_per_word[SPI_VOL_CTRL_1] = 32;
            _speed[SPI_VOL_CTRL_1] = 2000000;
            break;

        case SPI_VOL_CTRL_2:
            devspi = "/dev/spidev2.1";
            _mode[SPI_VOL_CTRL_2] = SPI_MODE_0;
            _bits_per_word[SPI_VOL_CTRL_2] = 32;
            _speed[SPI_VOL_CTRL_2] = 2000000;
            break;

        default:
            printf(">>can not find spidev\n");
            return -1;
    }

    _spifd[dn] =  spiOpen(devspi.c_str(), &_mode[dn], &_bits_per_word[dn], &_speed[dn]);

    return _spifd[dn];
}

int32 spiOpen(const char *devspi, uint8 *_mode, uint8 *_bits_per_word, uint32 *_speed)
{
    int32 res = 0;
    int32 fd = -1;

    Q_UNUSED(res);
    Q_UNUSED(devspi);
    Q_UNUSED(_mode);
    Q_UNUSED(_bits_per_word);
    Q_UNUSED(_speed);

#ifdef _ONPC_
    fd = 1;
#else
    fd = open(devspi, O_RDWR);
    if (fd < 0)
    {
        qDebug("cannot open spi device:%s\n",devspi);
        return -1;
    }

    res = ioctl(fd, SPI_IOC_WR_MODE, _mode);
    if (res == -1)
    {
        qDebug("can't set spi SPI_IOC_WR_MODE:0x%x\n", *_mode);
        return -1;
    }

    res = ioctl(fd, SPI_IOC_RD_MODE, _mode);
    if (res == -1)
    {
        qDebug("can't set spi SPI_IOC_RD_MODE:0x%x\n", *_mode);
        return -1;
    }
    /*
    * bits per word
    */
    res = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, _bits_per_word);
    if (res == -1)
    {
        qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:0x%x\n", *_bits_per_word);
        return -1;
    }

    res = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, _bits_per_word);
    if (res == -1)
    {
        qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:0x%x\n", *_bits_per_word);
        return -1;
    }

    /*
    * max speed hz
    */
    res = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, _speed);
    if (res == -1)
    {
        qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:%d\n", *_speed);
        return -1;
    }

    res = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, _speed);
    if (res == -1)
    {
        qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:%d\n", *_speed);
        return -1;
    }
#endif
    return fd;
}

int32 spiClose(int32 _spifd)
{
    int32 res = -1;
    Q_UNUSED(_spifd);
#ifndef _ONPC_
    res = close(_spifd);
    if(res < 0)
    {
        qDebug("error Spi close spifd=0x%x\n", _spifd);
        return -1;
    }
#endif
    return res;
}


volatile int32 ADC_Send(SPI_DEVICES dev, uint8 channel)
{
    //0x9247 / FFFF x 5 volts = 2.86V
    volatile uint16 readValue = 0;
    uint8 controlByte = 0;

    WriteGPIOPort(GPIO_5, 7, GPIO_LOW);
    WriteGPIOPort(GPIO_5, 8, GPIO_LOW);

    if(dev == SPI_ADS8343_A)
    {
        WriteGPIOPort(GPIO_2, 26, GPIO_LOW);
    }
    else if(dev == SPI_ADS8343_T)
    {
        WriteGPIOPort(GPIO_2, 27, GPIO_LOW);
    }
    else
    {
        return -1;
    }
#ifdef FEATURE_ADS8341
    controlByte = channel; //test(channel & 0x01)<<2 | (1 << (channel>>1));
#else //ADS8344
    controlByte = (channel & 0x01)<<2 | (channel & 0x06)>>1;
#endif
    _tx_buf[dev][0] = 0x87 | (controlByte & 0x07)<<4;
    _tx_buf[dev][1] = 0;
    _tx_buf[dev][2] = 0;
    _tx_buf[dev][3] = 0;
    _rx_buf[dev][0] = 0;
    _rx_buf[dev][1] = 0;
    _rx_buf[dev][2] = 0;
    _rx_buf[dev][3] = 0;

    //length 4 : command(8B) , receive1(8B), receive2(8B), receive3(8B), SPI_IOC_MESSAGE(1)
    spiWriteRead(dev, &_tx_buf[dev][0], &_rx_buf[dev][0], 4);

    WriteGPIOPort(gpioTools::GPIO_2, 27, gpioTools::GPIO_HIGH);
    WriteGPIOPort(gpioTools::GPIO_2, 26, gpioTools::GPIO_HIGH);

#ifdef FEATURE_ADS8341
    //AD8341
    readValue = (_rx_buf[dev][1] & 0x7F) << 9;
    readValue |= (_rx_buf[dev][2] & 0xFF) << 1;
    readValue |= (_rx_buf[dev][3] & 0x80) >> 7;
#else
    //VVR AD8343
    //5V : x = 65534 :
    readValue = (_rx_buf[dev][3] & 0xFF) << 8;
    readValue |= (_rx_buf[dev][2] & 0xFF);
#endif

#if 0 //hw test
    if(dev == SPI_ADS8343_A && channel == 0x01)
    {
        qDebug("cuurent: channel = %02X, 0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Current = %d",channel,_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
    }
    else if(dev == SPI_ADS8343_A && channel == 0x02)
    {
        qDebug("voltage: channel = %02X , 0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Voltage = %d",channel,_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
    }
    else if(dev == SPI_ADS8343_A && channel == 0x03)
    {
        qDebug("channel = %02X, :0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Voltage = %d",channel, _rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
    }
    else if(dev == SPI_ADS8343_A && channel == 0x04)
    {
        qDebug("channel = %02X,:0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Voltage = %d",channel,_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
    }
    else if(dev == SPI_ADS8343_A && channel == 0x05)
    {
        qDebug("channel = %02X,:0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Voltage = %d",channel,_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
    }

    /*
    else if(dev == SPI_ADS8343_T && channel == 0x05)
    {
        qDebug("0x%02x) 0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result Temp = %d",channel, _rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
    }
    */
    msDelay(1000);
#endif
#if 0
    if(dev == SPI_ADS8343_A && channel == 0x02)
    {

        qDebug("Temp_A:channel = %02X, 0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result Temp_A= %d",channel,_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
        //WriteGPIOPort(GPIO_3, 22, GPIO_HIGH);
    }
    msDelay(100);
    /*
    if(dev == SPI_ADS8343_A && channel == 0x01)
    {

        //qDebug("cuurent:0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Current = %d",_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
        WriteGPIOPort(GPIO_3, 22, GPIO_HIGH);
    }
    else
    {
        WriteGPIOPort(GPIO_3, 22, GPIO_LOW);
    }
    */
#endif
    return readValue;
}

void DAC_Send(uint16 data)
{

    //if(data == 0) WriteGPIOPortLow(GPIO_2, 22); //CLR
    //else WriteGPIOPortHigh(GPIO_2, 22);         //CLR

    //WriteGPIOPort(GPIO_2, 21, GPIO_HIGH);   //LD High
    //printf("DAC_Data = %d\n",data);
    WriteGPIOPort(GPIO_3, 19, GPIO_LOW);    //CS Low
    _tx_buf[SPI_DAC8830][0] = 0xff & data ;
    //_tx_buf[SPI_DAC8830][1] = 0x0f & (data>>8);
    _tx_buf[SPI_DAC8830][1] = 0xff & (data>>8);


    //length 2 : command1(8B) , command2(8B), SPI_IOC_MESSAGE(1)
    spiWrite(SPI_DAC8830, &_tx_buf[SPI_DAC8830][0], 2);
    //spiWrite(SPI_DAC8830, _tx_buf[SPI_DAC8830], 2);

    WriteGPIOPort(GPIO_3, 19, GPIO_HIGH);   //CS High
    //WriteGPIOPort(GPIO_3, 18, GPIO_LOW);    //LD Low
    //usleep(50);    //delay
    //WriteGPIOPort(GPIO_3, 18, GPIO_HIGH);    //LD High
}


void spiWrite(int32 fdId, volatile uint8 *tbuf, uint32 length)
{
    int32 ret = 0;
    Q_UNUSED(ret);
    memset(_io_trans, 0, sizeof(_io_trans));

    _io_trans[0].tx_buf = (unsigned long)tbuf;
    _io_trans[0].bits_per_word = _bits_per_word[fdId];
    _io_trans[0].speed_hz = _speed[fdId];
    _io_trans[0].len = length;

#ifndef _ONPC_
    ret = ioctl(_spifd[fdId], SPI_IOC_MESSAGE(1), &_io_trans);
    if(ret == 1)
    {
        printf("Cant send spi message=0x%x\n", _spifd[fdId]);
    }
    else if(ret == -1)
    {
        printf("Failed transferring data=%d\n", errno);
    }
#endif
}

void spiWriteRead(int32 fdId, volatile uint8 *tbuf, volatile uint8 *rbuf, uint32 length)
{
    int32 ret = 0;
    Q_UNUSED(ret);
    memset(_io_trans, 0, sizeof(_io_trans));

    _io_trans[0].tx_buf = (unsigned long)tbuf;
    _io_trans[0].rx_buf = (unsigned long)rbuf;
    _io_trans[0].bits_per_word = _bits_per_word[fdId];
    _io_trans[0].speed_hz = _speed[fdId];
    _io_trans[0].len = length;


#ifndef _ONPC_
    ret = ioctl(_spifd[fdId], SPI_IOC_MESSAGE(1), &_io_trans);

    if(ret == 1)
    {
        printf("Cant send spi message=0x%x\n", _spifd[fdId]);
    }
    else if(ret == -1)
    {
        printf("Failed transferring data=%d\n", errno);
    }

#else
#if test
    printf("fdId=%d, spi=%d, bps=%d, speed=%d, len=%d\n",fdId, _spifd[fdId], _bits_per_word[fdId], _speed[fdId], length );
#endif
#endif
    }

}
