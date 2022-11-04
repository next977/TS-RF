#ifndef SYSPARM_H
#define SYSPARM_H



#include <vector>
#include <QList>
#include "global.h"

//----------------------Main Check Impedance DAC ------------------//
#define CHECKIMPEDANCE_DAC_MIN 6000//6500 //min 100 ohm
#define CHECKIMPEDANCE_DAC_MAX 9200//8000 //max 6000 ohm
#define CHECKIMPEDANCE_DAC_MIDLE 6400//8000 //max 6000 ohm

//-----------------------Main Button time ms ----------------------//
#define RFSTARTBUTTON_TIME_MS 100//1000 //1000ms min
#define DCDC_RELAY_TIME_MS 620
#define CHECKIMPEDANCE_TIME_MS 200//1000 //1000ms min

using namespace std;

enum State_EN
{
    STATE_SYSTEM_STANDBY,
    STATE_SYSTEM_READY,
    STATE_SYSTEM_ON,
    STATE_SYSTEM_OFF,
    STATE_SYSTEM_ALARM,
    STATE_SYSTEM_ERROR,
    STATE_SCREEN_MAIN,
    STATE_SCREEN_SETUP
};
enum Alarm_Code
{
    ALARM_CODE_000,
    ALARM_CODE_001,
    ALARM_CODE_002,
    ALARM_CODE_003

};
enum Error_Code
{
    ERROR_CODE_000,
    ERROR_CODE_001,
    ERROR_CODE_002,
    ERROR_CODE_003

};

enum Mode_RF
{
    MODE_RF_PULSE = 0,
    MODE_RF_CONSTANT

};
enum State_Pad
{
    STATE_PAD_ON,
    STATE_PAD_OFF

};

enum State_Needle
{
    STATE_NEEDLE_ON,
    STATE_NEEDLE_OFF
};
enum Angle_Needle
{
    ANGLE_NEEDLE_50,
    ANGLE_NEEDLE_86,
    ANGLE_DEFAULT
};
enum Length_Needle
{
    LENGTH_NEEDLE_98,
    LENGTH_NEEDLE_89,
    LENGTH_NEEDLE_71,
    LENGTH_DEFAULT
};



enum BTNS
{
    BTN_RFSTARTSTOP = 0,
    BTN_MODE,
    BTN_GRAPH,
    BTN_SETTING,
    BTN_OHMECHECK,
    BTN_MAX,
    BTN_UP,
    BTN_DOWN
};
enum Click_State
{

    CLICK_NONE,
    CLICK_TIMER_DATA,
    CLICK_VOLUME,
    CLICK_DATE_DAY,
    CLICK_DATE_MONTH,
    CLICK_DATE_YEAR,
    CLICK_TIME_HOUR,
    CLICK_TIME_MINUTE,
    CLICK_TIME_AMPM,
    CLICK_BRIGHTNESS,

    CLICK_CALIBRATION_DAC_UNITS,
    CLICK_CALIBRATION_DAC_TENS,
    CLICK_CALIBRATION_DAC_HUNDREDS,
    CLICK_CALIBRATION_DAC_THOUSANDS,
    CLICK_CALIBRATION_DAC_TENTHOUSANDS,
    CLICK_CALIBRATION_DIVISION_V_UNITS,
    CLICK_CALIBRATION_DIVISION_V_TENS,
    CLICK_CALIBRATION_DIVISION_V_HUNDREDS,
    CLICK_CALIBRATION_DIVISION_I_UNITS,
    CLICK_CALIBRATION_DIVISION_I_TENS,
    CLICK_CALIBRATION_DIVISION_I_HUNDREDS,
    CLICK_CALIBRATION_AVERAGE_COUNT


};
enum Call_State
{
    CALL_FROME_MAINSYSTEM,
    CALL_FROME_SETUP,
    CALL_FROME_ENGINEER,
    CALL_FROME_CALIBRATION

};

enum RGB_State
{

    RGB_RED,
    RGB_GREEN,
    RGB_BLUE

};
enum Plot_State
{
    PLOT_MAIN_PRINTING,
    PLOT_MAIN_UPDATE,
    PLOT_SUB_PRINTING
};
enum LED_Color
{
    LED_COLOR_RED,
    LED_COLOR_GEERN,
    LED_COLOR_BLUE,
    LED_COLOR_WHITE,
    LED_COLOR_ORANGE,
    LED_COLOR_GRAY,
    LED_COLOR_YELLOW,
    LED_COLOR_DEFAULT
};
enum LED_Point
{
    LED_POINT_LEFT,
    LED_POINT_MIDLE,
    LED_POINT_RIGHT,
    LED_POINT_DEFAULT

};

enum Watt_Select
{
    WATT_7,
    WATT_15,
    WATT_25,
    WATT_50

};


typedef struct tagRfFactors
{
#if 1
    uint8 RfDeivceID; //RF Device ID

    uint8 tmMRFAState;
    uint8 tmAmplifierState;
    uint8 tmLastError;

    uint16 tmReadADCVoltage;
    uint16 tmReadADCCurrent;
    uint16 tmAverReadADCVoltage;
    uint16 tmAverReadADCCurrent;


    uint16 tmReadADCTemperatureA;
    uint16 tmReadADCTemperatureB;

    uint16 tmDACValueToSetLast;
    uint16 tmDACValueToSetSave;
    uint16 tmDACValueToSetforCal;



    uint16 rfLastCurrentMeasured;

    uint16 rfPowerMeasured;
    uint16 rfVoltageMeasured;
    uint16 rfCurrentMeasured;
    uint16 rfImpedanceMeasured;
    uint16 rfImpedanceMeasured_last;
    uint16 rfDACTableMeasured_last;

    uint16 rfImpedanceTableMeasured7;
    uint16 rfImpedanceTableMeasured15;
    uint16 rfImpedanceTableMeasured25;
    uint16 rfImpedanceTableMeasured50;
    uint16 rfDACTableMeasured7;
    uint16 rfDACTableMeasured15;
    uint16 rfDACTableMeasured25;
    uint16 rfDACTableMeasured50;


    uint16 rfTemperatureMeasuredA;
    uint16 rfTemperatureMeasuredB;
    uint16 rfTemperaturePreviousA;
    uint16 rfTemperaturePreviousB;

    uint16 tmTooHighImpedanceDelayTime;
    uint16 tmTooHighTemperatureDelayTime;

    uint32 tmMinImpedance;

    uint16 CheckImpedace_DAC;


    uint16 targetVMS;
    uint16 targetWatt;



    uint16 RMS_ADC_Volt;
    uint16 RMS_ADC_Current;
    
    uint16 v_division_level;
    uint16 i_division_level;
    uint16 average_count;

    uint8 SIC_page0[32];
    uint8 SIC_page1[32];
    bool SIC_result;
    bool SIC_Parsing;
    Watt_Select watt_select; //false : 25w , True : 50w
    uint32 mode_click_state;
    uint32 mode_state;
    uint32 counting;
    uint32 state;
    //bool  tmAmplifierActive;      //RF amplifer enable

    uint32 Temp_value;
    uint32 Fan_value;

//<------------------Date file----------------->

    uint32 Volume;
    uint32 Date_year;
    uint32 Date_month;
    uint32 Date_day;
    uint32 Time_hour;
    uint32 Time_min;
    uint32 Time_sec;
    uint32 Timer_countlevel;

    Click_State click_state;
    Mode_RF mode_rf;

    State_EN state_system;
    State_EN current_state_system;
    State_Needle state_needle;
    State_Pad state_pad;
//<-------------------------------------------->
//<------------------Data Graph----------------->

    QList<uint32> rf_Count;
    QList<uint32> rf_State;
    QList<uint32> rf_IM;
    QList<uint32> rf_VM;
    QList<uint32> rf_CM;
    QList<uint32> rf_PM;
    QList<uint32> rf_DAC;
    QList<uint32> rf_before_IM;
    QList<uint32> rf_before_PM;






    bool System_result;
    bool tmRollOff;
    bool isStepwise;
    bool tmTooHighTempeatureFlag;
    bool tmTooHighImpedanceFlag;

#endif
    uint16 rfSetPower;
    uint16 rfSetTime;

}RF_DEVICE_FACTORS;


class SysParm
{
public:
    const uint8 *SERIAL_NO;
    const uint8 *SW_VER;

    //en_RFDeviceOPMode 		tmOperationMode;

    typedef struct tagRFCTable
    {
        uint16 tmRealValue;
        uint16 tmDigitalValue;
    } RFC_TABLE;
    RFC_TABLE FileTransTableRFPowerToDAC[19][3];
    RFC_TABLE FileTransTableRFVoltage[19][3];
    RFC_TABLE FileTransTableRFCurrent[19][3];

    uint8      tmImpedanceControlMethod;

    uint16 tmRFPowerLevelPerModes[3] = {50, 50, 0};     //puncture, pulse, continuance // Watt
    uint16 tmRFPowerLevelExtPerModes[3] = {0, 0, 120};  //puncture, pulse, continuance // Temp

    uint32 tmRFCogluationTime_PerModes_C[3] = {10, 0, 25}; //puncture, pulse, continuance // Time Continue add
    uint32 tmRFCogluationTime_PerModes_P[3] = {0, 0, 0}; //puncture, pulse, continuance// Time Puncture add
    uint32 tmRFCgluationTimesPerModes[3] = {0, 0 ,0}; //puncture, pulse, continuance// Main Time sec save add

//    U32 tmRFSteptimePerModes[RF_DEVICE_OP_MODE_SIZE];

    uint32 tmElapsedTime;
    uint32 tmElapsedTime_count;
    uint32 Impedance_check_count;
    uint32 Impedance_pulse_count;
    uint32 pulse_count;

    uint32 tmBeforeElapsedTime;

    uint32 tmSystemElapsedTime;

    uint32 tmRFCogluationTime_Puncture;
    uint32 tmRFCogluationTime_Continue;
    uint32 tmRFCogluationTime_Pulse;
    uint32 tmRFCogluationTime_Idle;
    uint32 tmRFCogluationTime_PulseMode;


    uint32 tmRFCogluationTimeM;
    int32 modeSettingParam;


    uint32 File_TABLE_DEVICE_ID;
    uint32 File_TABLE_STEP_VOLTAGE;
    uint32 File_TABLE_STEP_CURRENT;
    uint32 File_TABLE_STEP_RFPOWER_REF;

    uint32 CQM_Data;
    uint32 CQM_Data_Average;
    uint32 CQM_Data_Sum;
    double CQM_Calculate;

    bool isDoTestforCal;

    RF_DEVICE_FACTORS *mRfDeviceFactors;
    vector <uint16> vRFPowerToDAC;
    vector <uint16> vRFVoltage;
    vector <uint16> vRFCurrent;
    vector <uint16> vImp_Correct7;
    vector <uint16> vImp_Correct15;
    vector <uint16> vImp_Correct25;
    vector <uint16> vImp_Correct50;

    vector <uint16> vDAC_Correct7;
    vector <uint16> vDAC_Correct15;
    vector <uint16> vDAC_Correct25; 
    vector <uint16> vDAC_Correct50;


    //vector <uint16> vTemperatureA;
    //vector <uint16> vTemperatureB;

public:
    SysParm();
    bool Inspect_table();
    bool MRFA_Table_open();
    ~SysParm();

    uint16 GetLastADVoltage(void) const {return lastADVoltage;}
    uint16 GetLastVMS(void) const {return lastVMS;}
    uint16 GetLastADCurrent(void) const {return lastADCurrent;}
    uint16 GetLastADTemperatureA(void) const {return lastADTemperatureA;}
    uint16 GetLastADTemperatureB(void) const {return lastADTemperatureB;}
    uint16 GetLastDACPower(void) const {return lastDACPower;}

    uint16 getLimitPower(void) const;
    uint16 getMinPower(void) const;
    uint16 GetWattToDACValuefor1Watt(void) const;

    bool doTestforCal(void) const {return isDoTestforCal;}
    void vector_at();
private:
    uint16 lastADVoltage;
    uint16 lastVMS;
    uint16 lastADCurrent;
    uint16 lastImp_Correct;
    uint16 lastDAC_Correct;

    uint16 lastADTemperatureA;
    uint16 lastADTemperatureB;
    uint16 lastADTemperatureBoard;
    uint16 lastDACPower;
    uint16 limitPower;
    uint16 minPower;
    uint16 dacValuefor1Watt;




};

#endif // SYSPARM_H
