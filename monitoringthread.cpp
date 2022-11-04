#include "monitoringthread.h"
#include <QElapsedTimer>
#include <thread>
#define __INTERVAL_MONITORING__ 10 //50

MonitoringThread::MonitoringThread(QObject *parent, SysParm *sysParm) : QThread(parent)
{
    this->sysParm = sysParm;

    Open_Device();
    SetDefault_value();
    SetDefault_function();
#ifndef _ONPC_
    SecuredIC = new securedic(this, sysParm);
    USBOTG = new usbotg(this);
    SetConnect();
#endif
}
void MonitoringThread::SetConnect()
{
    connect(this, SIGNAL(sig_SecuredIC_Start()), SecuredIC, SLOT(slot_SecuredIC_Start()));
}

void MonitoringThread::Open_Device()
{
#ifndef _ONPC_
    gpioTools::FAN_I2C_Open();
    gpioTools::LED_I2C_Open();
    gpioTools::Temp_TMP117_I2C_Open();
    gpioTools::SecuredIC_Open();
#endif

}

void MonitoringThread::SetDefault_value()
{
    LED_State = true;
    LED_Count = 0;
    LED_Brightness_State = true;
    LED_Brightness_Add = 0;
    LED_Brightness_Sub = 250;
    LED_Brightness_Count = 0;
    Temp_Count = 0;
    SIC_Count = 0;
    CQM_Count = 0;
    LED_Control_Count = 0;
}
void MonitoringThread::SetDefault_function()
{
#ifndef _ONPC_
    Set_Front_RGBLED();
    Set_CQM();
    gpioTools::FAN_Setting();
#endif
}
void MonitoringThread::Set_CQM()
{
#ifndef _ONPC_
    gpioTools::MAX1237_Write(0xAA); //extern clock A8
    gpioTools::MAX1237_Write(0x65); //AIN2
#endif
}
void MonitoringThread::Read_CQM()
{
    CQM_Count ++;
#if 0
    if(CQM_Count % 2 == 0) //50ms * 20 = 100ms
    {
        sysParm->CQM_Data_Sum += gpioTools::MAX1237_Read();
    }

    if(CQM_Count >= 20) //50ms * 200 = 1000ms
    {
#ifndef _ONPC_
        sysParm->CQM_Data = gpioTools::MAX1237_Read();
#endif
        sysParm->CQM_Data_Average = sysParm->CQM_Data_Sum /10;
        sysParm->CQM_Data_Sum = 0;g
        CQM_Count = 0;
    }
#endif
    sysParm->CQM_Data = gpioTools::MAX1237_Read();
    sysParm->CQM_Data_Sum += sysParm->CQM_Data;

    if(CQM_Count >= 50) //10ms * 50 = 500ms
    {
        sysParm->CQM_Data_Average = sysParm->CQM_Data_Sum /50;
        sysParm->CQM_Calculate = CQM_calculate(sysParm->CQM_Data_Average);
        //qDebug("CQM_Calculate = %lf",CQM_Calculate(sysParm->CQM_Data_Average));
        sysParm->CQM_Data_Sum = 0;
        CQM_Count = 0;
    }
}
double MonitoringThread::CQM_calculate(uint32 CQM_Data)
{
    double a,b,c;
    double result;
    uint32 x = CQM_Data;

    if(CQM_Data <= 2700)
    {
        a = 0.000036;
        b = 0.025427;
        c = 31.537559;
#if 0
        qDebug("a = %lf",a);
        qDebug("b = %lf",b);
        qDebug("c = %lf",c);
#endif
        result = a*(x*x)+b*x+c;
    }
    else if(CQM_Data > 2700)
    {
        a = 0.001969;
        b = -10.198162;
        c = 13540.903859;
#if 0
        qDebug("a = %lf",a);
        qDebug("b = %lf",b);
        qDebug("c = %lf",c);
#endif
        result = a*(x*x)+b*x+c;
    }
    return result;
}

void MonitoringThread::Set_Front_RGBLED()
{
    qDebug("Set Front_RGBLED");
#ifndef _ONPC_
    gpioTools::LP5009_Write(gpioTools::LED_COMMANDS_DEVICE_CONFIG0, 0x40);
#endif
}
void MonitoringThread::State_Change_RGBLED()
{
    if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY)
    {
        color_state = LED_COLOR_BLUE;
        point_state = LED_POINT_MIDLE;
#ifndef _ONPC_
        //Control_SIC();
#endif
    }
    else if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY)
    {
        color_state = LED_COLOR_GEERN;
        point_state = LED_POINT_MIDLE;
#ifndef _ONPC_
        //Control_SIC();
#endif
    }
    else if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ON)
    {
        color_state = LED_COLOR_YELLOW; //LED_COLOR_WHITE
    }
    else if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ALARM)
    {
        color_state = LED_COLOR_ORANGE;
        point_state = LED_POINT_MIDLE;
    }
    else if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ERROR)
    {
        color_state = LED_COLOR_RED;
        point_state = LED_POINT_MIDLE;
    }

    Control_RGBLED();
}

void MonitoringThread::Control_RGBLED()
{
    if(color_state == LED_COLOR_YELLOW) //LED_COLOR_WHITE
    {
        if(LED_State == true) //LED On
        {
            Display_Front_RGBLED(color_state, LED_POINT_LEFT,  LED_Brightness_Add);
            Display_Front_RGBLED(color_state, point_state, LED_Brightness_Sub);
            Display_Front_RGBLED(color_state, LED_POINT_RIGHT, LED_Brightness_Add);

        }
        else
        {
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_LEFT,  0);
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_MIDLE, 0);
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_RIGHT, 0);
        }
    }
    else if(color_state == LED_COLOR_ORANGE || color_state == LED_COLOR_RED) //Alarm or Error
    {
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_LEFT,  0);
            Display_Front_RGBLED(color_state, point_state, 0);
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_RIGHT, 0);
    }
    else
    {
        if(LED_State == true) //LED On
        {
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_LEFT,  0);
            Display_Front_RGBLED(color_state, point_state, 0);
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_RIGHT, 0);
            if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY ||
               sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ON )
            {
#ifndef _ONPC_
                gpioTools::RFSwitchLED_ON();
#endif
            }

        }
        else //LED Off
        {
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_LEFT,  0);
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_MIDLE, 0);
            Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_RIGHT, 0);
#ifndef _ONPC_
            gpioTools::RFSwitchLED_OFF();
#endif
        }
    }

#if 1
    if(LED_Brightness_State == true)
    {
        LED_Brightness_Count += 50;
        LED_Brightness_Add += 50;
        LED_Brightness_Sub -= 50;

    }
    else
    {
        LED_Brightness_Count -= 50;
        LED_Brightness_Add -= 50;
        LED_Brightness_Sub += 50;
    }
    if(LED_Brightness_Count >= 250)
    {
       LED_Brightness_State = false;
    }
    else if (LED_Brightness_Count <= 0)
    {
       LED_Brightness_State = true;
    }
#endif
    LED_Count ++;
    if(color_state != LED_COLOR_YELLOW && LED_Count >= 20) //LED_COLOR_WHITE
    {
        LED_State ^= true;
        LED_Count = 0;
    }
    else if (color_state == LED_COLOR_YELLOW && LED_Count >= 40) //LED_COLOR_WHITE
    {
        LED_State = false;
        LED_Count = 0;
    }
}

void MonitoringThread::Display_Front_RGBLED(LED_Color color, LED_Point point, int32 brightness)
{
    //qDebug("slot_LED_Display Test");
    uint32 point_arr[3];
#ifndef _ONPC_
#if 0
    gpioTools::LED_I2C_Open();
    gpioTools::LP5009_Write(gpioTools::LED_COMMANDS_OUT0_COLOR, 0xFF);
    gpioTools::LP5009_Write(gpioTools::LED_COMMANDS_OUT3_COLOR, 0xFF);
    gpioTools::LP5009_Write(gpioTools::LED_COMMANDS_OUT6_COLOR, 0xFF);
    gpioTools::LED_I2C_Close();
#endif
    //qDebug("LED Color = %d, LED Point = %d", color ,point );
    switch(point)
    {
        case LED_POINT_LEFT:
                point_arr[0] = gpioTools::LED_COMMANDS_OUT0_COLOR;
                point_arr[1] = gpioTools::LED_COMMANDS_OUT1_COLOR;
                point_arr[2] = gpioTools::LED_COMMANDS_OUT2_COLOR;

            break;
        case LED_POINT_MIDLE:
                point_arr[0] = gpioTools::LED_COMMANDS_OUT3_COLOR;
                point_arr[1] = gpioTools::LED_COMMANDS_OUT4_COLOR;
                point_arr[2] = gpioTools::LED_COMMANDS_OUT5_COLOR;
            break;
        case LED_POINT_RIGHT:
                point_arr[0] = gpioTools::LED_COMMANDS_OUT6_COLOR;
                point_arr[1] = gpioTools::LED_COMMANDS_OUT7_COLOR;
                point_arr[2] = gpioTools::LED_COMMANDS_OUT8_COLOR;
            break;
        default :
            point_arr[0] = 0xff;
            point_arr[1] = 0xff;
            point_arr[2] = 0xff;
            break;
    }

    /*
    if(State != true)
    {
        color = LED_COLOR_DEFAULT;
    }
    */
    switch(color)
    {
        case LED_COLOR_WHITE:
                gpioTools::LP5009_Write(point_arr[0], brightness); //0xFF
                gpioTools::LP5009_Write(point_arr[1], brightness); //0xFF
                gpioTools::LP5009_Write(point_arr[2], brightness); //0xFF
            break;
        case LED_COLOR_GEERN:
                gpioTools::LP5009_Write(point_arr[0], 0x00);
                gpioTools::LP5009_Write(point_arr[1], 0xFF);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;
        case LED_COLOR_BLUE:
                gpioTools::LP5009_Write(point_arr[0], 0x00);
                gpioTools::LP5009_Write(point_arr[1], 0x00);
                gpioTools::LP5009_Write(point_arr[2], 0xFF);
            break;

        case LED_COLOR_ORANGE:
                gpioTools::LP5009_Write(point_arr[0], 0xFF);
                gpioTools::LP5009_Write(point_arr[1], 0x5E);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;

        case LED_COLOR_RED:
                gpioTools::LP5009_Write(point_arr[0], 0xFF);
                gpioTools::LP5009_Write(point_arr[1], 0x00);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;
        case LED_COLOR_GRAY:
                gpioTools::LP5009_Write(point_arr[0], 0x32);
                gpioTools::LP5009_Write(point_arr[1], 0x32);
                gpioTools::LP5009_Write(point_arr[2], 0x32);
            break;
        case LED_COLOR_YELLOW:
                gpioTools::LP5009_Write(point_arr[0], brightness); //0xFF
                gpioTools::LP5009_Write(point_arr[1], brightness); //0xFF
                gpioTools::LP5009_Write(point_arr[2], 0); //0xFF
            break;
        case LED_COLOR_DEFAULT: //LED Off
                gpioTools::LP5009_Write(point_arr[0], 0x00);
                gpioTools::LP5009_Write(point_arr[1], 0x00);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;

        default :
            break;
    }

#endif

}
void MonitoringThread::SIC_Control_Function()
{

    uint32 CheckID_Length = 4;
    uint32 Lot_Length = 15;
    uint32 Type1_Length = 2;
    uint32 Type2_Length = 2;
    uint32 Reserved_Length = 2;

    uint8 CheckID[CheckID_Length];
    uint8 Lot[Lot_Length];
    uint8 Type1[Type1_Length];
    uint8 Type2[Type2_Length];
    uint8 Reserved[Reserved_Length];
    Length_Needle Needle_length;
    Angle_Needle Needle_angle;
    if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY ||
       sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY)
    {
        //qDebug("SIC Read Start!");
        emit sig_SecuredIC_Start();
    }
#if 1
    if(sysParm->mRfDeviceFactors->SIC_result == true && sysParm->mRfDeviceFactors->SIC_Parsing == true)
    {
        for(int i = 0; i < CheckID_Length; i++)
        {
            CheckID[i] = sysParm->mRfDeviceFactors->SIC_page0[i];
        }
        if(CheckID[0] == 'S' &&
           CheckID[1] == 'T' &&
           CheckID[2] == 'N' &&
           CheckID[3] == 'D' )
        {
            for(int i = 0; i < Lot_Length; i++)
            {
                Lot[i] = sysParm->mRfDeviceFactors->SIC_page0[i+CheckID_Length];
            }
            for(int i = 0; i < Type1_Length; i++)
            {
                Type1[i] = sysParm->mRfDeviceFactors->SIC_page0[i+CheckID_Length+Lot_Length];
                //qDebug("Type1[%d] = %c", i , Type1[i]);
            }
            for(int i = 0; i < Type2_Length; i++)
            {
                Type2[i] = sysParm->mRfDeviceFactors->SIC_page0[i+CheckID_Length+Lot_Length+Type1_Length];
                //qDebug("Type2[%d] = %c", i , Type2[i]);
            }
            for(int i = 0; i < Reserved_Length; i++)
            {
                Reserved[i] = sysParm->mRfDeviceFactors->SIC_page0[i+CheckID_Length+Lot_Length+Type1_Length+Type2_Length];
            }
            //Set_State_Needle(STATE_NEEDLE_ON);
            if(Type1[0] == '9'&&
               Type1[1] == '8')
            {
               Needle_length = LENGTH_NEEDLE_98;
            }
            else if(Type1[0] == '8'&&
                    Type1[1] == '9')
            {
               Needle_length = LENGTH_NEEDLE_89;
            }
            else if(Type1[0] == '7'&&
                    Type1[1] == '1')
            {
               Needle_length = LENGTH_NEEDLE_71;
            }

            if(Type2[0] == '5'&&
               Type2[1] == '0')
            {
               Needle_angle = ANGLE_NEEDLE_50;
            }
            else if(Type2[0] == '8'&&
                    Type2[1] == '6')
            {
               Needle_angle = ANGLE_NEEDLE_86;
            }

            emit sig_Set_Needle(STATE_NEEDLE_ON , Needle_length, Needle_angle );
            emit sig_State_check();
            sysParm->mRfDeviceFactors->SIC_Parsing = false;
            qDebug("needle connect");

        }

    }
    else if(sysParm->mRfDeviceFactors->state_needle == STATE_NEEDLE_ON &&
            sysParm->mRfDeviceFactors->SIC_result == false &&
            sysParm->mRfDeviceFactors->SIC_Parsing == true)
    {
        //Set_State_Needle(STATE_NEEDLE_OFF);
        emit sig_Set_Needle(STATE_NEEDLE_OFF , LENGTH_DEFAULT, ANGLE_DEFAULT );
        emit sig_State_check();
        qDebug("needle out");
    }
#endif
}
void MonitoringThread::SIC_Control()
{
    SIC_Count ++;
    if(SIC_Count >= 30) //10ms * 30 = 300ms
    {
#ifndef _ONPC_
        SIC_Control_Function();
#endif
        SIC_Count = 0;
    }

}

void MonitoringThread::Temp_Fan_Control()
{
    Temp_Count ++;
    if(Temp_Count >= 100) //10ms * 100 = 1s
    {
#ifndef _ONPC_
        Temp_Fan_Function();
        //qDebug("Temp Fan Function In");
        //Setting_RTCVolume();
#endif
        Temp_Count = 0;
    }
}

void MonitoringThread::Temp_Fan_Function()
{

    QString temp_hex, temp_dec;
    unsigned char* temp_str;
    uint32 temp_sum, temp_result;
    bool ok;

#ifndef _ONPC_
    temp_str = gpioTools::TMP117_Read(gpioTools::TEMP_TMP117_COMMANDS_TEMP_RESULT);

    //qDebug("temp_result[0] = %02X", temp_str[0]);
    //qDebug("temp_result[1] = %02X", temp_str[1]);

    temp_hex.sprintf("%02X%02X", temp_str[0],temp_str[1]);
    //sscanf(temp, "%d", temp_result);
    ok = true;
    temp_sum= temp_hex.toUInt(&ok, 16);
    //qDebug("temp_sum = %d", temp_sum);
    temp_result = temp_sum/128;
    temp_dec.sprintf("%d", temp_result);
    //qDebug("temp_result = %d", temp_result);

    //ui->T_hex_lcd->display(temp_hex);
    //ui->T_dec_lcd->display(temp_dec);

    sysParm->mRfDeviceFactors->Temp_value = temp_result;
    if(sysParm->mRfDeviceFactors->Fan_value != 0)
    {
        /*
        if(sysParm->mRfDeviceFactors->Fan_value > sysParm->mRfDeviceFactors->Temp_value )
        {
            gpioTools::FAN_Write(0);
        }
        else
        {
            gpioTools::FAN_Write(sysParm->mRfDeviceFactors->Fan_value); //28
        }
        */
        if(sysParm->mRfDeviceFactors->Fan_value < sysParm->mRfDeviceFactors->Temp_value )
        {
            gpioTools::FAN_Write(sysParm->mRfDeviceFactors->Fan_value); //28
        }
        else if(sysParm->mRfDeviceFactors->Fan_value < sysParm->mRfDeviceFactors->Temp_value +2)
        {

        }
        else
        {
            gpioTools::FAN_Write(0);
        }

    }

#endif

}


void MonitoringThread::run()
{
    while(1)
    {
        QElapsedTimer _mTimer;
        QElapsedTimer _countTimer;
        _mTimer.start();
        _countTimer.start();
#ifndef _ONPC_
        /////////////////Start Sensor///////////////////
        Temp_Fan_Control();
        SIC_Control();
        Read_CQM();
        LED_Control_Count ++;
        if(LED_Control_Count >= 5)
        {
            State_Change_RGBLED();
            LED_Control_Count = 0;
        }
        ////////////////end Sensor///////////////////////
#endif
        waitTime = __INTERVAL_MONITORING__ - _mTimer.elapsed();    // 30ms 32 - 34 per 1 second to the pc
        //waitTime = outputMode->Interbal_Monitoring - _mTimer.elapsed();
        //qDebug("%d",waitTime);
        if(waitTime >= 0)
        {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli> (waitTime));
        }

       // qDebug() << "thread";
       // qDebug("%d",_countTimer.elapsed());
    }
}
