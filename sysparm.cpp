#include "sysparm.h"
#include "mrfa_rf_table.h"
#include <QDebug>

extern const uint8 SERIAL_NO[];
extern const uint8 SW_VER[];

SysParm::SysParm()
{
    mRfDeviceFactors = new RF_DEVICE_FACTORS;

    mRfDeviceFactors->tmReadADCCurrent = 0;
    mRfDeviceFactors->tmReadADCVoltage = 0;
    mRfDeviceFactors->tmAverReadADCCurrent = 0;
    mRfDeviceFactors->tmAverReadADCVoltage = 0;
    mRfDeviceFactors->tmReadADCTemperatureA = 0;
    mRfDeviceFactors->tmReadADCTemperatureB = 0;
    mRfDeviceFactors->rfImpedanceMeasured_last = 0;

    mRfDeviceFactors->targetVMS=0;
    mRfDeviceFactors->targetWatt=0;
    mRfDeviceFactors->counting = 0;


    mRfDeviceFactors->tmDACValueToSetLast = 0;
    mRfDeviceFactors->tmDACValueToSetforCal = 0;

    mRfDeviceFactors->CheckImpedace_DAC = 0;


    mRfDeviceFactors->v_division_level= 88;
    mRfDeviceFactors->i_division_level= 77;

    tmElapsedTime= 0;
    tmElapsedTime_count = 0;
    tmBeforeElapsedTime = 0;
    tmSystemElapsedTime= 0;


    tmRFCogluationTime_Puncture = 0;
    tmRFCogluationTime_Continue = 0;
    tmRFCogluationTime_Pulse = 0;
    tmRFCogluationTime_Idle = 0;
    tmRFCogluationTime_PulseMode = 0;

    CQM_Data = 0;
    CQM_Data_Average = 0;
    CQM_Data_Sum = 0;
    CQM_Calculate = 0;

    mRfDeviceFactors->Volume = 4; //0~63
    mRfDeviceFactors->Fan_value = 28; //temp value : 25
    mRfDeviceFactors->Timer_countlevel = 2;
    mRfDeviceFactors->average_count = 7;
    tmElapsedTime = 0;
    isDoTestforCal = false;


    mRfDeviceFactors->SIC_result = false;
    mRfDeviceFactors->SIC_Parsing = true;
    for(int i = 0; i < 32; i ++)
    {
        mRfDeviceFactors->SIC_page0[i] = 0;
        mRfDeviceFactors->SIC_page1[i] = 0;

    }

    mRfDeviceFactors->watt_select = WATT_7; // false : 25w , true : 50w
//    this->SERIAL_NO = (const U8*)&SERIAL_NO[0];
//    this->SW_VER = (const U8*)&SW_VER[0];


    //MRFA_Table_open();


    mRfDeviceFactors->rfSetPower = 0;
    mRfDeviceFactors->rfSetTime = 0;

    Inspect_table();

}

SysParm::~SysParm()
{
    if(mRfDeviceFactors)
        delete mRfDeviceFactors;
}

uint16 SysParm::getMinPower() const
{
    return minPower;
}

uint16 SysParm::GetWattToDACValuefor1Watt() const
{
    return dacValuefor1Watt;
}

uint16 SysParm::getLimitPower() const
{
    return limitPower;
}
#if 0
void SysParm::vector_at()
{
    limitPower = vRFPowerToDAC.at(TABLE_STEP_RFPOWER_REF-11);
    minPower = vRFPowerToDAC.at(14);

    for(uint32 i=0; i<TABLE_STEP_RFPOWER_REF; i++)
    {
        if(vRFPowerToDAC.at(TABLE_STEP_RFPOWER_REF+i) >= 10 )
        {
            dacValuefor1Watt = vRFPowerToDAC.at(i);
            break;
        }
    }
}

bool SysParm::MRFA_Table_open()
{
    /*
    if()
    {
        qCritical("error MRFA_Table_Open_Fail");
        return false;
    }
    */

    uint16 v1, v2, v3, v4, v5, i;

    v1 = v2 = v3 = v4 = v5 = 0;

    for (i = 0; i < TABLE_STEP_RFPOWER_REF; i++)
    {
        if(v1 >  FileTransTableRFPowerToDAC[i][0].tmDigitalValue)
        {
            qCritical("error FileTransTableRFPowerToDAC_DigitalValue");
            return false;
        }
        v1 = FileTransTableRFPowerToDAC[i][0].tmDigitalValue;
        vRFPowerToDAC.push_back(v1);
    }
    lastDACPower = static_cast<uint16>(v1);
    //lastDACPower = v1;

    v1 = 0;
    for (i = 0; i < TABLE_STEP_RFPOWER_REF; i++)
    {
        if(v1 >  FileTransTableRFPowerToDAC[i][0].tmRealValue)
        {
            qCritical("error FileTransTableRFPowerToDAC_RealValue");
            return false;
        }
        v1 = FileTransTableRFPowerToDAC[i][0].tmRealValue;
        vRFPowerToDAC.push_back(v1);
    }



    for (i = 0; i < TABLE_STEP_VOLTAGE; i++)
    {
        if(v2 >  FileTransTableRFVoltage[i][0].tmDigitalValue )
        {
            qCritical("error FileTransTableRFVoltage_DigitalValue");
            return false;
        }
        v2 = FileTransTableRFVoltage[i][0].tmDigitalValue;
        vRFVoltage.push_back(v2);
    }
    lastADVoltage = static_cast<uint16>(v2);
    v2 = 0;
    for (i = 0; i < TABLE_STEP_VOLTAGE; i++)
    {
        if(v2 >  FileTransTableRFVoltage[i][0].tmRealValue )
        {
            qCritical("error FileTransTableRFVoltage_RealValue");
            return false;
        }
        v2 = FileTransTableRFVoltage[i][0].tmRealValue;
        vRFVoltage.push_back(v2);
    }
    lastVMS = v2;

    for (i = 0; i < TABLE_STEP_CURRENT; i++)
    {
        if(v3 >  FileTransTableRFCurrent[i][0].tmDigitalValue )
        {
            qCritical("error FileTransTableRFCurrent_DigitalValue");
            return false;
        }
        v3 = FileTransTableRFCurrent[i][0].tmDigitalValue;
        vRFCurrent.push_back(v3);
    }
    lastADCurrent = static_cast<uint16>(v3);
    v3=0;
    for (i = 0; i < TABLE_STEP_CURRENT; i++)
    {
        if(v3 >  FileTransTableRFCurrent[i][0].tmRealValue)
        {
            qCritical("error FileTransTableRFCurrent_RealValue");
            return false;
        }
        v3 = FileTransTableRFCurrent[i][0].tmRealValue;
        vRFCurrent.push_back(v3);
    }
    mRfDeviceFactors->rfLastCurrentMeasured = static_cast<uint16>(v3);
    v3=0;


    vector_at();
    return true;

}
#endif
bool SysParm::Inspect_table()
{
    uint16 v1,v2, i, compare;

    v1 = 0;
    v2 = 0;
    compare = 0;
#if 0
    for (i = 0; i < TABLE_STEP_RFPOWER_REF; i++)
    {
        if(v1 >  TransTableRFPowerToDAC[i][0].tmDigitalValue)
        {
            qCritical("error TransTableRFPowerToDAC");
            return false;
        }
        v1 = TransTableRFPowerToDAC[i][0].tmDigitalValue;
        vRFPowerToDAC.push_back(v1);
    }
    lastDACPower = static_cast<uint16>(v1);
    //lastDACPower = v1;

    v1 = 0;
    for (i = 0; i < TABLE_STEP_RFPOWER_REF; i++)
    {
        if(v1 >  TransTableRFPowerToDAC[i][0].tmRealValue)
        {
            qCritical("error TransTableRFPowerToDAC");
            return false;
        }
        v1 = TransTableRFPowerToDAC[i][0].tmRealValue;
        vRFPowerToDAC.push_back(v1);
    }



    for (i = 0; i < TABLE_STEP_VOLTAGE; i++)
    {
        if(v2 >  TransTableRFVoltage[i][0].tmDigitalValue )
        {
            qCritical("error TransTableRFVoltage");
            return false;
        }
        v2 = TransTableRFVoltage[i][0].tmDigitalValue;
        vRFVoltage.push_back(v2);
    }
    lastADVoltage = static_cast<uint16>(v2);
    v2 = 0;
    for (i = 0; i < TABLE_STEP_VOLTAGE; i++)
    {
        if(v2 >  TransTableRFVoltage[i][0].tmRealValue )
        {
            qCritical("error TransTableRFVoltage");
            return false;
        }
        v2 = TransTableRFVoltage[i][0].tmRealValue;
        vRFVoltage.push_back(v2);
    }
    lastVMS = v2;

    for (i = 0; i < TABLE_STEP_CURRENT; i++)
    {
        if(v3 >  TransTableRFCurrent[i][0].tmDigitalValue )
        {
            qCritical("error TransTableRFCurrent");
            return false;
        }
        v3 = TransTableRFCurrent[i][0].tmDigitalValue;
        vRFCurrent.push_back(v3);
    }
    lastADCurrent = static_cast<uint16>(v3);
    v3=0;
    for (i = 0; i < TABLE_STEP_CURRENT; i++)
    {
        if(v3 >  TransTableRFCurrent[i][0].tmRealValue)
        {
            qCritical("error TransTableRFCurrent");
            return false;
        }
        v3 = TransTableRFCurrent[i][0].tmRealValue;
        vRFCurrent.push_back(v3);
    }
    mRfDeviceFactors->rfLastCurrentMeasured = static_cast<uint16>(v3);
    v3=0;
#endif
    ////////////////////////////7w////////////////////////////
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][3].tmDigitalValue)
        {
            qCritical("error TransTableImpedance DigitalValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][3].tmDigitalValue;
        vImp_Correct7.push_back(v1);
    }
    //lastImp_Correct = static_cast<uint16>(v1);
    v1=0;
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][3].tmRealValue)
        {
            qCritical("error TransTableImpedance RealValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][3].tmRealValue;
        vImp_Correct7.push_back(v1);
    }
    ///////////////////////////15w/////////////////////////
    v1=0;
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][2].tmDigitalValue)
        {
            qCritical("error TransTableImpedance DigitalValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][2].tmDigitalValue;
        vImp_Correct15.push_back(v1);
    }
    //lastImp_Correct = static_cast<uint16>(v1);
    v1=0;
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][2].tmRealValue)
        {
            qCritical("error TransTableImpedance RealValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][2].tmRealValue;
        vImp_Correct15.push_back(v1);
    }

    v1=0;
    ////////////////////////////25w////////////////////////////
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][0].tmDigitalValue)
        {
            qCritical("error TransTableImpedance DigitalValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][0].tmDigitalValue;
        vImp_Correct25.push_back(v1);
    }
    //lastImp_Correct = static_cast<uint16>(v1);
    v1=0;
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][0].tmRealValue)
        {
            qCritical("error TransTableImpedance RealValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][0].tmRealValue;
        vImp_Correct25.push_back(v1);
    }
    ///////////////////////////50w/////////////////////////
    v1=0;
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][1].tmDigitalValue)
        {
            qCritical("error TransTableImpedance DigitalValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][1].tmDigitalValue;
        vImp_Correct50.push_back(v1);
    }
    //lastImp_Correct = static_cast<uint16>(v1);
    v1=0;
    for (i = 0; i < TABLE_STEP_IMP_CORRECT; i++)
    {
        if(v1 >  TransTableImp_Correct[i][1].tmRealValue)
        {
            qCritical("error TransTableImpedance RealValue");
            return false;
        }
        v1 = TransTableImp_Correct[i][1].tmRealValue;
        vImp_Correct50.push_back(v1);
    }



    ////////////////////////////7w////////////////////////////
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(v2 >  TransTableDAC_Correct[i][3].tmDigitalValue)
        {
            qCritical("error TransTableDAC DigitalValue");
            return false;
        }
        v2 = TransTableDAC_Correct[i][3].tmDigitalValue;
        vDAC_Correct7.push_back(v2);
        if(v2 == 3100)
        {
           compare = i;
        }
    }
    //lastDAC_Correct = static_cast<uint16>(v2);
    v2=0;
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(i <= compare)
        {
            if(v2 >  TransTableDAC_Correct[i][3].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        else
        {
            if(v2 <  TransTableDAC_Correct[i][3].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        v2 = TransTableDAC_Correct[i][3].tmRealValue;
        vDAC_Correct7.push_back(v2);
    }

    ////////////////////////////15w////////////////////////////
    compare = 0;
    v2=0;
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(v2 >  TransTableDAC_Correct[i][2].tmDigitalValue)
        {
            qCritical("error TransTableDAC DigitalValue");
            return false;
        }
        v2 = TransTableDAC_Correct[i][2].tmDigitalValue;
        vDAC_Correct15.push_back(v2);
        if(v2 == 1500)
        {
           compare = i;
        }
    }
    //lastDAC_Correct = static_cast<uint16>(v2);
    v2=0;
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(i <= compare)
        {
            if(v2 >  TransTableDAC_Correct[i][2].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        else
        {
            if(v2 <  TransTableDAC_Correct[i][2].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        v2 = TransTableDAC_Correct[i][2].tmRealValue;
        vDAC_Correct15.push_back(v2);
    }



    compare = 0;
    v2=0;
    ////////////////////////////25w////////////////////////////
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(v2 >  TransTableDAC_Correct[i][0].tmDigitalValue)
        {
            qCritical("error TransTableDAC DigitalValue");
            return false;
        }   
        v2 = TransTableDAC_Correct[i][0].tmDigitalValue;
        vDAC_Correct25.push_back(v2);
        if(v2 == 3000)
        {
           compare = i;
        }
    }
    //lastDAC_Correct = static_cast<uint16>(v2);
    v2=0;
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(i <= compare)
        {
            if(v2 >  TransTableDAC_Correct[i][0].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        else
        {
            if(v2 <  TransTableDAC_Correct[i][0].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        v2 = TransTableDAC_Correct[i][0].tmRealValue;
        vDAC_Correct25.push_back(v2);
    }

    ////////////////////////////50w////////////////////////////
    compare = 0;
    v2=0;
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(v2 >  TransTableDAC_Correct[i][1].tmDigitalValue)
        {
            qCritical("error TransTableDAC DigitalValue");
            return false;
        }
        v2 = TransTableDAC_Correct[i][1].tmDigitalValue;
        vDAC_Correct50.push_back(v2);
        if(v2 == 1500)
        {
           compare = i;
        }
    }
    //lastDAC_Correct = static_cast<uint16>(v2);
    v2=0;
    for (i = 0; i < TABLE_STEP_DAC_CORRECT; i++)
    {
        if(i <= compare)
        {
            if(v2 >  TransTableDAC_Correct[i][1].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        else
        {
            if(v2 <  TransTableDAC_Correct[i][1].tmRealValue)
            {
                qCritical("error TransTableDAC RealValue");
                return false;
            }
        }
        v2 = TransTableDAC_Correct[i][1].tmRealValue;
        vDAC_Correct50.push_back(v2);
    }

   // mRfDeviceFactors->rfLastCurrentMeasured = static_cast<uint16>(v1);

#if 0
    for (i = 0; i < TABLE_STEP_TEMPERATURE; i++)
    {
        if(v5 >  TransTableTemperature[i][1].tmDigitalValue)
        {
            qCritical("error TransTableTemperature");
            return false;
        }
        v5 = TransTableTemperature[i][1].tmDigitalValue;
        vTemperatureB.push_back(v5);
    }
    lastADTemperatureB = static_cast<uint16>(v5);
    v5=0;
    for (i = 0; i < TABLE_STEP_TEMPERATURE; i++)
    {
        if(v5 >  TransTableTemperature[i][1].tmRealValue)
        {
            qCritical("error TransTableTemperature");
            return false;
        }
        v5 = TransTableTemperature[i][1].tmRealValue;
        vTemperatureB.push_back(v5);
    }



    vector_at();
#endif
    return true;
}

