 #include "rfoutputmodes.h"
#include <string>
#include <math.h>
#include "QDebug"


#define CALCULATE_DAC(cVMS, cDAC, tVMS) (cDAC*tVMS/cVMS)

/*
 *
 * PULSE MODE
 *
 */

PulseMode::PulseMode(SysParm *sysParm)  : OutputMode (sysParm)
{
}

PulseMode::~PulseMode()
{
}

const int8* PulseMode::getModeTitle()
{
    return "PULSE";
}

unsigned short PulseMode::getVMSforWatt(unsigned short power, unsigned short imp)
{
    double c_power = static_cast<double>(power);
    double vms = sqrt(c_power * static_cast<double>(imp)) * 10;

    if(vms > 2120) //sysParm->GetLastVMS())
        return 2120; //sysParm->GetLastVMS();
    else
         return static_cast<uint16>(vms);
}

unsigned int PulseMode::getTableIndexforVMS(unsigned int data)
{
    unsigned int m_len = sysParm->vRFVoltage.size();
    unsigned int low = m_len/2;
    unsigned int high = m_len;
    unsigned int j = 0;

    while (high - low > 1) {
        unsigned int probe = (low + high) / 2;
        unsigned int v = sysParm->vRFVoltage[probe];
        if (v > data)
            high = probe;
        else
            low = probe;
    }

    if (high == m_len)
        j = m_len;
    else
        j = high;

    return j;
}

void PulseMode::RFStart()
{
    pulse_sub = 0;

}
void PulseMode::RFStop()
{

}

void PulseMode::MRFA_DoTestImpedanceState()
{

}
uint16 PulseMode::getVMSDAC_Index(uint16 impedance)
{
    uint16 resultIndex;
    uint16 DivImpedance = impedance/100;

    if(DivImpedance <= 20)
    {
        if(DivImpedance == 0)
        {
            resultIndex = DivImpedance;
        }
        else
        {
            resultIndex = DivImpedance-1;
        }
    }
    else if(DivImpedance <=24)
    {
        resultIndex = 20;
    }
    else if(DivImpedance <=29)
    {
        resultIndex = 21;
    }
    else if(DivImpedance <=34)
    {
        resultIndex = 22;
    }
    else if(DivImpedance <=39)
    {
        resultIndex = 23;
    }
    else if(DivImpedance <=44)
    {
        resultIndex = 24;
    }
    else if(DivImpedance <=49)
    {
        resultIndex = 25;
    }
    else
    {
        resultIndex = 26;
    }
#if 0
    qDebug("DivImpedance = %d , resultIndex = %d",DivImpedance ,resultIndex);
#endif
    return resultIndex;
}

void PulseMode::SafetyDutyCycle()
{
    qDebug("Pulse Mode - SafetyDutyCycle State");
    qDebug("Counting %d",sysParm->mRfDeviceFactors->counting);
    //qDebug("sysParm->tmElapsedTime_count = %d",sysParm->tmElapsedTime_count);

    if(sysParm->tmElapsedTime_count >= pulse_sub )
    {
        sysParm->mRfDeviceFactors->counting = 0;
        sysParm->Impedance_check_count = 0;
        sysParm->Impedance_pulse_count ++;
        sysParm->mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_IMPEDANCE_CHECKING;

        return ;
    }
    sysParm->mRfDeviceFactors->counting ++;
}

void PulseMode::RFPowerControl() //300ms output , 700ms huge
{

    qDebug("Pulse Mode - RFPower Contol State");
    qDebug("Counting %d",sysParm->mRfDeviceFactors->counting);
    qDebug("sysParm->tmElapsedTime_count = %d",sysParm->tmElapsedTime_count);

    if(sysParm->tmElapsedTime_count >= 30 + pulse_sub)
    {
        pulse_sub += 100+(sysParm->pulse_count*10) ; //0, 1,2   //0,10,20
        sysParm->mRfDeviceFactors->counting = 0;
        sysParm->mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_READY;
        if(sysParm->pulse_count < sysParm->mRfDeviceFactors->Timer_countlevel)
        {
           sysParm->pulse_count++;
        }
        return ;
    }


    DACPortOutputSet(mRfDeviceFactors->rfDACTableMeasured_last);
    sysParm->mRfDeviceFactors->counting ++;
}
/*
 *
 * CONSTANT MODE
 *
 */

using namespace std;

ConstantMode::ConstantMode(SysParm *sysParm)  : OutputMode (sysParm)
{

}

ConstantMode::~ConstantMode()
{
}

const int8* ConstantMode::getModeTitle()
{
    return "Constant";
}

unsigned short ConstantMode::getVMSforWatt(unsigned short power, unsigned short imp)
{
    double c_power = static_cast<double>(power);
    double vms = sqrt(c_power * static_cast<double>(imp)) * 10;

    if(vms > 2120) //sysParm->GetLastVMS())
        return 2120; //sysParm->GetLastVMS();
    else
         return static_cast<uint16>(vms);
}

unsigned int ConstantMode::getTableIndexforVMS(unsigned int data)
{
    unsigned int m_len = sysParm->vRFVoltage.size();
    unsigned int low = m_len/2;
    unsigned int high = m_len;
    unsigned int j = 0;

    while (high - low > 1) {
        unsigned int probe = (low + high) / 2;
        unsigned int v = sysParm->vRFVoltage[probe];
        if (v > data)
            high = probe;
        else
            low = probe;
    }

    if (high == m_len)
        j = m_len;
    else
        j = high;

    return j;
}

void ConstantMode::RFStart()
{
    last_impedance_state = true;
}
void ConstantMode::RFStop()
{

}
void ConstantMode::MRFA_DoTestImpedanceState()
{

}
void ConstantMode::SafetyDutyCycle()
{

}
uint16 ConstantMode::getVMSDAC_Index(uint16 impedance)
{
    uint16 resultIndex;
    uint16 DivImpedance = impedance/100;

    if(DivImpedance <= 20)
    {
        if(DivImpedance == 0)
        {
            resultIndex = DivImpedance;
        }
        else
        {
            resultIndex = DivImpedance-1;
        }
    }
    else if(DivImpedance <=24)
    {
        resultIndex = 20;
    }
    else if(DivImpedance <=29)
    {
        resultIndex = 21;
    }
    else if(DivImpedance <=34)
    {
        resultIndex = 22;
    }
    else if(DivImpedance <=39)
    {
        resultIndex = 23;
    }
    else if(DivImpedance <=44)
    {
        resultIndex = 24;
    }
    else if(DivImpedance <=49)
    {
        resultIndex = 25;
    }
    else
    {
        resultIndex = 26;
    }
#if 0
    qDebug("DivImpedance = %d , resultIndex = %d",DivImpedance ,resultIndex);
#endif
    return resultIndex;
}


void ConstantMode::RFPowerControl()
{
    //qDebug("Constant Mode RFPower Contol State");
    //qDebug("Counting %d",sysParm->mRfDeviceFactors->counting);
    uint16 VMS_V;
    uint16 VMSDAC_Index;
    uint16 VMSDAC_value;
    uint16 Impedance;
    //uint32 TargetDAC;
    uint16 TargetDAC;

    qDebug("Constant Mode - RFPower Contol State");
    qDebug("Counting %d",sysParm->mRfDeviceFactors->counting);
#if 0
    if(!(mRfDeviceFactors->rfVoltageMeasured >=275 || mRfDeviceFactors->rfCurrentMeasured >=500))
    {
        //mRfDeviceFactors->targetWatt = (mRfDeviceFactors->targetWatt + mRfDeviceFactors->rfPowerLevel)/2;
        mRfDeviceFactors->targetWatt = 25;
        if(last_impedance_state == true)
        {
            Impedance = mRfDeviceFactors->rfImpedanceMeasured_last;
            last_impedance_state = false;
        }
        else
        {
            Impedance = mRfDeviceFactors->rfImpedanceMeasured;
        }

        VMSDAC_Index= getVMSDAC_Index(Impedance);
        VMS_V = sqrt(mRfDeviceFactors->targetWatt*Impedance);
        VMSDAC_value = VMSDAC_Value_array[VMSDAC_Index];
        TargetDAC = VMS_V*VMSDAC_value;

        if(TargetDAC >= 32767) //47500
        {
            TargetDAC = 32767; //47500
        }
        qDebug("Im = %d",Impedance);
        qDebug("Tw = %d",mRfDeviceFactors->targetWatt);
        qDebug("VMSDAC_value = %d",VMSDAC_value);
        qDebug("VMS_V = %d",VMS_V);
        qDebug("TargetDAC = %d",TargetDAC);
    }
    else
    {
        TargetDAC = 0;
    }
#endif
#if 0
    if(sysParm->mRfDeviceFactors->watt_select == WATT_7)
    {
        TargetDAC = sysParm->mRfDeviceFactors->rfDACTableMeasured7;
    }
    else if(sysParm->mRfDeviceFactors->watt_select == WATT_15)
    {
        TargetDAC = sysParm->mRfDeviceFactors->rfDACTableMeasured15;
    }
    else if(sysParm->mRfDeviceFactors->watt_select == WATT_25)
    {
        TargetDAC = sysParm->mRfDeviceFactors->rfDACTableMeasured25;
    }
    else if(sysParm->mRfDeviceFactors->watt_select == WATT_50)
    {
        TargetDAC = sysParm->mRfDeviceFactors->rfDACTableMeasured50;
    }
    DACPortOutputSet(TargetDAC); //32767
#endif
    DACPortOutputSet(mRfDeviceFactors->rfDACTableMeasured_last);
    sysParm->mRfDeviceFactors->counting ++;
}
