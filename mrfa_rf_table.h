#ifndef __MRFA_RF_TABLE__
#define __MRFA_RF_TABLE__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct tagRFCTable
{
    uint16 tmRealValue;
    uint16 tmDigitalValue;
} RFC_TABLE;
#define TABLE_DEVICE_TESTID 4
#define TABLE_DEVICE_ID 3
#define TABLE_STEP_VOLTAGE 62
#define TABLE_STEP_CURRENT 62
#define TABLE_STEP_RFPOWER_REF 62
#define TABLE_STEP_TEMPERATURE 19
#define TABLE_STEP_IMP_CORRECT 50
#define TABLE_STEP_DAC_CORRECT 50

#define TEMPERATURE_MAX 1300//990

#define MAX_ADC_TEMPERATURE 	TransTableTemperature[TABLE_STEP_TEMPERATURE-1][0].tmDigitalValue//3254	//-----ADC value at 100 C'-----modified090512
#define MIN_ADC_TEMPERATURE 	TransTableTemperature[0][0].tmDigitalValue//3254	//-----ADC value at 100 C'-----modified090512
#define MIN_ADC_CURRENT 		TransTableRFCurrent[5][0].tmDigitalValue//5487	//-----ADC value at 100 mA-----modified090512
#define LIMIT_POWER				TransTableRFPowerToDAC[TABLE_STEP_RFPOWER_REF-11][0].tmDigitalValue
#define MAX_DAC_POWER			TransTableRFPowerToDAC[TABLE_STEP_RFPOWER_REF-1][0].tmDigitalValue

//->->->->->->->->->->->->->->->->->->->->->->-> VIVA BD ->->->->->->->->->->->->->->->->->->->->->->->//
#define TABLE_TEMP_CHANNEL_ID 4

#define MAX_ADC_TEMPERATURE1 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][0].tmDigitalValue
#define MAX_ADC_TEMPERATURE2 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][1].tmDigitalValue
#define MAX_ADC_TEMPERATURE3 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][2].tmDigitalValue
#define MAX_ADC_TEMPERATURE4 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][3].tmDigitalValue
#define MIN_ADC_TEMPERATURE1 	TransTableVASCABTemperature[0][0].tmDigitalValue
#define MIN_ADC_TEMPERATURE2 	TransTableVASCABTemperature[0][1].tmDigitalValue
#define MIN_ADC_TEMPERATURE3 	TransTableVASCABTemperature[0][2].tmDigitalValue
#define MIN_ADC_TEMPERATURE4 	TransTableVASCABTemperature[0][3].tmDigitalValue

#define TEST_ADC_TEMPERATURE1 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][0].tmDigitalValue
#define TEST_ADC_TEMPERATURE2 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][1].tmDigitalValue
#define TEST_ADC_TEMPERATURE3 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][2].tmDigitalValue
#define TEST_ADC_TEMPERATURE4 	TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE-1][3].tmDigitalValue

//->->->->->->->->->->->->->->->->->->->->->->-> VIVA BD ->->->->->->->->->->->->->->->->->->->->->->->//


//추가부분 10.08.03
#define MAX_ADC_VOLTAGE			TransTableRFVoltage[TABLE_STEP_VOLTAGE-1][0].tmDigitalValue
#define TEST_ADC_VOLTAGE		TransTableRFVoltage[12][0].tmDigitalValue
#define MAX_ADC_CURRENT			TransTableRFCurrent[TABLE_STEP_CURRENT-1][0].tmDigitalValue
#define TEST_ADC_CURRENT		TransTableRFCurrent[12][0].tmDigitalValue
#define TEST_ADC_TEMPERATURE 	TransTableTemperature[TABLE_STEP_TEMPERATURE-1][0].tmDigitalValue
#define TEST_POWER				10
#define TEST_DUMMY_LOAD			100


extern const RFC_TABLE TransTableRFPowerToDAC[TABLE_STEP_RFPOWER_REF][TABLE_DEVICE_ID];
extern const RFC_TABLE TransTableRFCurrent[TABLE_STEP_CURRENT][TABLE_DEVICE_ID];
extern const RFC_TABLE TransTableRFVoltage[TABLE_STEP_VOLTAGE][TABLE_DEVICE_ID];
//extern const RFC_TABLE TransTableTemperature[TABLE_STEP_TEMPERATURE][TABLE_DEVICE_ID];
extern const RFC_TABLE TransTableImp_Correct[TABLE_STEP_IMP_CORRECT][TABLE_DEVICE_TESTID];
extern const RFC_TABLE TransTableDAC_Correct[TABLE_STEP_DAC_CORRECT][TABLE_DEVICE_TESTID];


//->->->->->->->->->->->->->->->->->->->->->->-> VIVA BD ->->->->->->->->->->->->->->->->->->->->->->->//
extern const RFC_TABLE TransTableVASCABTemperature[TABLE_STEP_TEMPERATURE][TABLE_TEMP_CHANNEL_ID];
//->->->->->->->->->->->->->->->->->->->->->->-> VIVA BD ->->->->->->->->->->->->->->->->->->->->->->->//



extern const uint8 SERIAL_NO[];
extern const uint8 SW_VER[];



#ifdef __cplusplus
}
#endif


#endif
