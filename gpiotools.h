#ifndef GPIOTOOLS_H
#define GPIOTOOLS_H

#include "global.h"
#include <QThread>

#define mdelay(n) QThread::msleep(n)
#define udelay(n) QThread::usleep(n)

#define I2C_ADDR_MAX9768 0x4b //MAX9768 Volume Slave Address (Chip Address)
#define I2C_ADDR_MAX31343 0x68 //MAX31343 RTC Slave Address (Chip Address)
#define I2C_ADDR_PT7C4339  0x68 //PT7C4339 RTC Slave Address (Chip Address)
#define I2C_ADDR_SHT40 0x44 //SHT40-AD1B-R2 TEMP Slave Address (Chip Address)
#define I2C_ADDR_MAX31760 0x50 //MAX31760 FAN Slave Address (Chip Address)
#define I2C_ADDR_SECUREDIC 0x18 //Secured IC Slave Address (Chip Address)
#define I2C_ADDR_LP5009 0x14 //LP5009 LED Slave Address (Chip Address)
#define I2C_ADDR_TMP117 0x49 //TEMP117 Temp Slave Address (Chip Address)
#define I2C_ADDR_MAX1237 0x34 //MAX1237 CQM Slave Address (Chip Address)


// defines the data direction (reading from I2C device) in I2C_start(),I2C_rep_start()
#define I2C_READ    1

// defines the data direction (writing to I2C device) in I2C_start(),I2C_rep_start()
#define I2C_WRITE   0

// Flags on I2C_read
#define ACK    1
#define NACK   0

#define EXPECT_ACK    1
#define EXPECT_NACK   0

#define POLL_LIMIT  200

// block modes
#define MODE_WRITEBLOCK     1
#define MODE_READBLOCK      2
#define MODE_WRITEREADBLOCK 3
#define MODE_WRITEPOLL      4
// misc constants


namespace gpioTools {


    enum SPI_DEVICES {SPI_DAC7611 = 0, SPI_DAC8830, SPI_ADS8343_A, SPI_ADS8343_T, SPI_VOL_CTRL_1, SPI_VOL_CTRL_2, ALL_SPI_DEVICES};
    enum SEND_OLED_TYPE{OLED_CMD, OLED_DATA};

    enum en_GPIOPort
    {
        GPIO_1,
        GPIO_2,
        GPIO_3,
        GPIO_4,
        GPIO_5,
        GPIO_6,
        GHIO_LAST
    };

    enum ELECTRODE_Type_en
    {
        ELECTRODE_NONE = 0,
        ELECTRODE_MONOPOLAR,
        ELECTRODE_BI_3CM,
        ELECTRODE_BI_7CM
    };

    enum RF_OutputDir_en
    {
        RFOUTPUT_DIR_DUMMY,
        RFOUTPUT_DIR_TARGET
    };


    enum LED_Output_en
    {
        Leds_HANDLE_LED,
    };
//<--------------------LED----------------------->
    enum LED_Color
    {
        LED_COLOR_RED,
        LED_COLOR_GEERN,
        LED_COLOR_BLUE,
        LED_COLOR_WHITE,
        LED_COLOR_ORANGE,
        LED_COLOR_DEFAULT
    };
    enum LED_Point
    {
        LED_POINT_LEFT,
        LED_POINT_MIDLE,
        LED_POINT_RIGHT,
        LED_POINT_DEFAULT

    };
//<------------------------------------------->
    enum I2C_State
    {
        I2C_STATE_RTC,
        I2C_STATE_VOLUME,
        I2C_STATE_TEMP,
        I2C_STATE_FAN,
        I2C_STATE_SIC,
        I2C_STATE_LED,
        I2C_STATE_CQM

    };
//<--------------------RTC & Volume----------------------->

    enum OM_State
    {
        OM_CLASSICPWM = 0xdf,
        OM_FLITERLESS = 0xd6
    };

    enum RTC_REGISTERS_State
    {
        RTC_REGISTERS_SECONDS = 0x06,
        RTC_REGISTERS_MINUTES = 0x07,
        RTC_REGISTERS_HOURS = 0x08,
        //REGISTERS_DFTW = 0x03,
        RTC_REGISTERS_DAY = 0x09,
        RTC_REGISTERS_DATES = 0x0A,
        RTC_REGISTERS_MONTHS = 0x0B,
        RTC_REGISTERS_YEARS = 0x0C,
        RTC_REGISTERS_ALARM1_SECONDS = 0x0D,
        RTC_REGISTERS_ALARM1_MINUTES = 0x0E,
        RTC_REGISTERS_ALARM1_HOURS = 0x0F,
        RTC_REGISTERS_ALARM1_DAYDATE = 0x10,
        RTC_REGISTERS_ALARM2_MINUTES = 0x13,
        RTC_REGISTERS_ALARM2_HOURS = 0x14,
        RTC_REGISTERS_ALARM2_DAYDATE = 0x15,
        RTC_REGISTERS_PWR_MGMT = 0x18,
        RTC_REGISTERS_TRICKLE_REG = 0x19
        //REGISTERS_CONTROL = 0x0E,
        //REGISTERS_STATUS = 0x0F,
        //REGISTERS_TC = 0x10
    };


//<---------------------------------------------------->

//<--------------------Temp TMP117 I2C----------------------->
        enum TEMP_TMP117_COMMANDS_State
        {
            TEMP_TMP117_COMMANDS_TEMP_RESULT = 0x00,
            TEMP_TMP117_COMMANDS_CONFIGURATION = 0x01,
            TEMP_TMP117_COMMANDS_THIGH_LIMIT = 0x02,
            TEMP_TMP117_COMMANDS_TLOW_LIMIT = 0x03,
            TEMP_TMP117_COMMANDS_TEMP_OFFSET = 0x07
        };

//<---------------------------------------------------->
//<--------------------Temp SHT40 I2C----------------------->
        enum TEMP_SHT40_COMMANDS_State
        {
            TEMP_SHT40_COMMANDS_HIGH_T_RH = 0xFD,
            TEMP_SHT40_COMMANDS_MEDIUM_T_RH = 0xF6,
            TEMP_SHT40_COMMANDS_LOWEST_T_RH = 0xE0

        };

//<---------------------------------------------------->
    enum LED_COMMANDS_State
    {
        LED_COMMANDS_DEVICE_CONFIG0 = 0x00,
        LED_COMMANDS_DEVICE_CONFIG1 = 0x01,
        LED_COMMANDS_OUT0_COLOR = 0x0B, //LED0 - Red
        LED_COMMANDS_OUT1_COLOR = 0x0C, //LED0 - Green
        LED_COMMANDS_OUT2_COLOR = 0x0D, //LED0 - Blue
        LED_COMMANDS_OUT3_COLOR = 0x0E, //LED1 - Red
        LED_COMMANDS_OUT4_COLOR = 0x0F, //LED1 - Green
        LED_COMMANDS_OUT5_COLOR = 0x10, //LED1 - Blue
        LED_COMMANDS_OUT6_COLOR = 0x11, //LED2 - Red
        LED_COMMANDS_OUT7_COLOR = 0x12, //LED2 - Green
        LED_COMMANDS_OUT8_COLOR = 0x13 //LED2 - Blue

    };

//<--------------------FAN I2C----------------------->
    enum FAN_COMMANDS_State
    {

        FAN_REGISTERS_CR1 = 0x00,
        FAN_REGISTERS_CR2 = 0x01,
        FAN_REGISTERS_CR3 = 0x02,
        FAN_REGISTERS_FFDC = 0x03,
        FAN_REGISTERS_MASK = 0x04,
        FAN_REGISTERS_IFR = 0x05,
        FAN_REGISTERS_RHSH = 0x06,
        FAN_REGISTERS_RHSL = 0x07,
        FAN_REGISTERS_LOTSH = 0x08,
        FAN_REGISTERS_LOTSL = 0x09,
        FAN_REGISTERS_ROTSH = 0x0A,
        FAN_REGISTERS_ROTSL = 0x0B,
        FAN_REGISTERS_LHSH = 0x0C,
        FAN_REGISTERS_LHSL = 0x0D,

        FAN_REGISTERS_PWMR = 0x50,
        FAN_REGISTERS_TCTH = 0x0E,
        FAN_REGISTERS_TCTL = 0x0F,

        FAN_REGISTERS_TC1H = 0x52,
        FAN_REGISTERS_TC1L = 0x53,
        FAN_REGISTERS_RTH = 0x56,
        FAN_REGISTERS_RTL = 0x57,
        FAN_REGISTERS_LTH = 0x58,
        FAN_REGISTERS_LTL = 0x59

    };

//<---------------------------------------------------->



    #define DACPortOutput(a) DAC_Send(a)

    static const uint32 GPIO_HIGH = 1;
    static const uint32 GPIO_LOW = 0;

#ifndef _ONPC_
    int32 uart_init();
    void uart_printf(char *pcFmt,...);
    void uart_binwrite(unsigned char *data, int length);
    char* uart_binread(int *length);
    void uart_close();
#endif
    void gpio_init();
    volatile unsigned *GetEncorderAddress();
    void WriteGPIOPort(uint32 gpio, uint32 pin, bool flag);
    void WriteGPIOPortHigh(uint32 port_id, uint32 port_value);
    void WriteGPIOPortLow(uint32 port_id, uint32 port_value);



    uint32 ReadGPIOPort(uint32 gpio, uint32 pin);

    void AudioAMPControl_Enable();
    void AudioAMPControl_Disable();
    void LCDBackLight_ON();
    void LCDBackLight_OFF();


    void EnableAmplifierPowerOutput();
    void DisableAmplifierPowerOutput();

    void EnableIsolated_DCDC();
    void DisableIsolated_DCDC();

    void EnableTargetRelay_1st();
    void EnableTargetRelay_2nd();
    void DisableTargetRelay_1st();
    void DisableTargetRelay_2nd();


    void RFSwitchLED_ON();
    void RFSwitchLED_OFF();

    void SetElectrodeType(ELECTRODE_Type_en eType);
    void SetAmplifierPowerOutputDirection(RF_OutputDir_en dir);
    void OnLED(LED_Output_en id);
    void OffLED(LED_Output_en id);

    void StopSound();
    void StartSound(uint32 id);

    void spi_init();
    void spiAllclose();

    int spiOpen(SPI_DEVICES dn);
    int spiOpen(const int8 *devspi, uint8 *_mode, uint8 *_bits_per_word, uint32 *_speed);
    void spiWrite(int32 fdId, volatile uint8 *tbuf, uint32 length);
    void spiWriteRead(int32 fdId, volatile uint8 *tbuf, volatile uint8 *rbuf, uint32 length);
    int spiClose(int32 _spifd);


    volatile int32 ADC_Send(SPI_DEVICES dev, uint8 channel);
    void DAC_Send(uint16 data);



//<--------------------RTC & Volume----------------------->
    void msDelay(int delay);
    int RTC_Open(void);
    int Volume_Open(void);
    int Temp_SHT40_I2C_Open(void);
    int Temp_TMP117_I2C_Open(void);
    int FAN_I2C_Open(void);
    int LED_I2C_Open(void);
    int CQM_I2C_Open(void);


    int RTC_Close(void);
    int Volume_Close(void);
    int Temp_SHT40_I2C_Close(void);
    int Temp_TMP117_I2C_Close(void);
    int FAN_I2C_Close(void);
    int LED_I2C_Close(void);
    int CQM_I2C_Close(void);

    void FAN_RPM_Read();
    unsigned char FAN_Temp_Read();
    void FAN_Setting();
    void FAN_Write(unsigned int fan_temp_value );
    void PWM_RGB(int RGB_Color, int dutycycle_count);


    int MAX9768_Write(uint32 OM, uint32 DataValue);
    int MAX31760_Write(uint32 Registers , uint32 DataValue);
    int PT7C4339_Write(uint32 Registers , uint32 DataValue);
    uint32 PT7C4339_Read(uint32 Registers);
    int MAX31343_Write(uint32 Registers , uint32 DataValue);
    unsigned char MAX31343_Read(uint32 REG_ADDR);
    int MAX31760_Write(uint32 Registers , uint32 DataValue);
    unsigned char MAX31760_Read(uint32 Registers);
    int MAX1237_Write(uint32 WriteData);
    unsigned short MAX1237_Read();

    int SHT40_Write(uint32 Address , uint32 Command );
    uint32 SHT40_Read(uint32 Registers);
    int LP5009_Write(uint32 Registers , uint32 DataValue);
    unsigned char* TMP117_Read(uint32 Registers);

    // I2C primitives


    void I2C_adapterReset(void);
    int I2C_start();
    void I2C_stop(void);
    int I2C_write(uchar data, int expect_ack);
    bool I2C_Write(I2C_State state, unsigned char *data, int length);

    unsigned char* I2C_Read(I2C_State state, int ack, int *result);
    uchar I2C_read(int ack, int *result);
    //unsigned I2C_read(int ack, int *result);


    int I2C_rep_start();
    int I2C_set_speed(int speed);
    // I2C high level functions
    int I2C_writeBlock(int i2c_addr, int addr, int block_len, uchar *block);
    int I2C_readBlock(int i2c_addr, int addr, int block_len, uchar *block, int skip_set_pointer);
    int I2C_writeReadBlock(int i2c_addr, int addr, int write_len, uchar *write_block,
                           int read_len, uchar *read_block);
    int I2C_readBlockPoll(int i2c_addr, int write_len, uchar *write_block,
                           uchar ones_mask, uchar zeros_mask, uchar *rslt);
    // I2C Utility functions

    int SecuredIC_Open(void);
    int SecuredIC_Close(void);
    int I2C_readAdapterVersion(uchar *version);





    // CRC8
    static unsigned char docrc8(unsigned char value);
    static unsigned char CRC8;
//<------------------------------------------------------>
}
#endif // GPIOTOOLS_H
