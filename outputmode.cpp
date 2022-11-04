#include "outputmode.h"
#include "gpiotools.h"

using namespace std;

OutputMode::OutputMode(SysParm *sysParm)
{
    this->sysParm = sysParm;
    this->mRfDeviceFactors = sysParm->mRfDeviceFactors;
}

OutputMode::~OutputMode()
{

}

void OutputMode::MRFA_DoTestImpedanceState()
{
#if 0
    if( mRfDeviceFactors->rfPowerLevel > 0 //change 5W->30W->15W(080131)->5W(101220)
           && mRfDeviceFactors->tmRFonTime > 1)//기본적으로 Power 값이 5이상으로 나오기 위해서는 2초 이하의 시간이 걸린다.
    {

        if( mRfDeviceFactors->tmMinImpedance  > mRfDeviceFactors->rfImpedanceMeasured )
        {
            mRfDeviceFactors->tmMinImpedance  = mRfDeviceFactors->rfImpedanceMeasured;
            return;
        }

        if(mRfDeviceFactors->tmMinImpedance < 50)
        {
            smTestImpedanceValueToChange = 30;
            tmImpedanceFactor = 0;
        }
        else
        {
            smTestImpedanceValueToChange = static_cast<uint32>(mRfDeviceFactors->tmMinImpedance* tmImpedanceChangeRatio/100);//modified(080131)
            tmImpedanceFactor = (mRfDeviceFactors->rfImpedanceMeasured < mRfDeviceFactors->tmMinImpedance) ? 0 : (mRfDeviceFactors->rfImpedanceMeasured - mRfDeviceFactors->tmMinImpedance);
        }
    }
#endif
}
void OutputMode::SafetyDutyCycle()
{

}

void OutputMode::RFPowerControlforCal()
{

}


void OutputMode::DACPortOutputInc(uint16 value)
{


    if(mRfDeviceFactors->tmDACValueToSetLast + value > sysParm->GetLastDACPower())
    {
        mRfDeviceFactors->tmDACValueToSetLast = sysParm->GetLastDACPower();
    }
    else
    {
        mRfDeviceFactors->tmDACValueToSetLast += value;
    }

    gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);

}

void OutputMode::DACPortOutputDec(uint16 value)
{


    if(value == 0 || mRfDeviceFactors->tmDACValueToSetLast < value)
    {
        mRfDeviceFactors->tmDACValueToSetLast = 0;
    }
    else
    {
        mRfDeviceFactors->tmDACValueToSetLast -= value;
    }

    gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);
}

void OutputMode::DACPortOutputSet(uint16 value)
{
    mRfDeviceFactors->tmDACValueToSetLast = value;

    gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);
}
