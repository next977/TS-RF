
#include "outputthread.h"
#include "gpiotools.h"
#include "rfoutputmodes.h"
#include <QElapsedTimer>

using namespace gpioTools;


#define __INTERVAL_MONITORING__ 10//18//23//9   //30ms, 33 cycle
#define __INTERVAL_TEST__ false

OutputThread::OutputThread(QObject *parent, SysParm *sysParm) : QObject (parent), WorkerThread ()
{
    //slot signals
#if __INTERVAL_TEST__
    checkpersec = new QTimer;
    counter = 0;
    connect(checkpersec, SIGNAL(timeout()), this, SLOT(slot_dispCounter()));
    checkpersec->start(1000);
#endif
    this->sysParm = sysParm;
    this->mRfDeviceFactors = sysParm->mRfDeviceFactors;
    monitoringTask = new MonitoringTask(this, sysParm);
    vModes.push_back(new PulseMode(sysParm));
    vModes.push_back(new ConstantMode(sysParm));

    outputMode = vModes.at(OutputMode::RF_DEVICE_OP_MODE_PULSE);

    RFStop_State = false;
    RF_Relay_State = false;
    RF_Relay_Count = 0;
    tmRFPowerActiveState = RF_ACTIVE_OFF;
    this->mRfDeviceFactors->CheckImpedace_DAC = CHECKIMPEDANCE_DAC_MIN;

    this->mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_READY;
    this->mRfDeviceFactors->tmDACValueToSetLast = 0;

    //SIC_Count = 0;

#ifndef _ONPC_
    outputMode->DACPortOutputDec(0);
#endif

}

OutputThread::~OutputThread()
{
    for(vector<OutputMode *>::iterator iter = vModes.begin(); iter != vModes.end(); )
    {
        OutputMode *modes = *iter;
        delete modes;
        ++iter;
    }

    if(monitoringTask) delete monitoringTask;
}


void OutputThread::loop()
{
    QElapsedTimer _mTimer;
    QElapsedTimer _countTimer;
    _mTimer.start();
    _countTimer.start();
    /*
     *  Start call function.
     *  Synchronous call function 일반적인 처리는 동기로
     *  Sensing adc
     */
    monitoringTask->GetAmplifierParameters();

    //Calibration mode

    //Impedance check

    //rf output for mode
    if(tmRFPowerActiveState == RF_ACTIVE_ON && RF_Relay_State == true)
    {
            if(RFStop_State == true)
            {
                RFStop_State = false;
                RF_Relay_State = false;
                //RFStop();

            }
            if(GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_PULSE) //Pulse Mode
            {
                if( sysParm->tmElapsedTime_count >= sysParm->mRfDeviceFactors->Timer_countlevel*100
                                                    +sysParm->Impedance_pulse_count*10 ) //x=10,20,30
                {
                        RF_Relay_State = false;
                        //RFStop();
                        qDebug("Pulse Mode Timer Count Over RF Stop");
                        return;
                }
            }
            else //Constant Mode
            {
                if( sysParm->tmElapsedTime >= sysParm->mRfDeviceFactors->Timer_countlevel )
                {
                        RF_Relay_State = false;
                        //RFStop();
                        qDebug("Constant Mode Timer Count Over RF Stop");
                        return;
                }
            }

            //RF output
            switch(mRfDeviceFactors->tmMRFAState)
            {
                        case OutputMode::RF_STATE_READY:
                            //outputMode->DACPortOutputSet(100);
                            outputMode->DACPortOutputSet(0);
#ifndef __TSRF_TEST__
                            qDebug("%d , %d , %d , %d , %d , %d , %d , %d , %d ",
                                   mRfDeviceFactors->counting, mRfDeviceFactors->state, mRfDeviceFactors->targetWatt, mRfDeviceFactors->targetVMS,
                                   mRfDeviceFactors->rfVoltageMeasured, mRfDeviceFactors->rfImpedanceMeasured,
                                   mRfDeviceFactors->rfCurrentMeasured, mRfDeviceFactors->rfPowerMeasured/100,
                                   mRfDeviceFactors->tmDACValueToSetLast);
#endif
                            outputMode->SafetyDutyCycle();

                            break;
                        case OutputMode::RF_STATE_STOP:
                            if( mRfDeviceFactors->rfCurrentMeasured < (150-20) )
                            {
                                outputMode->DACPortOutputInc(OutputMode::MRFA_AMPLIFIER_PARAM_2);//MRFA_AMPLIFIER_PARAM_DAC_FAST_INCREMENT;
                            }
                            else if( mRfDeviceFactors->rfCurrentMeasured > (150+10) )//(current+20) )
                            {
                                outputMode->DACPortOutputDec(OutputMode::MRFA_AMPLIFIER_PARAM_2);
                            }

                            break;
                        case OutputMode::RF_STATE_IMPEDANCE_MONITORING:
                            if( mRfDeviceFactors->rfCurrentMeasured < (170-20) )
                            {
                                outputMode->DACPortOutputInc(OutputMode::MRFA_AMPLIFIER_PARAM_2);//MRFA_AMPLIFIER_PARAM_DAC_FAST_INCREMENT;
                            }
                            else if( mRfDeviceFactors->rfCurrentMeasured > (170+10) )//(current+20) )
                            {
                                outputMode->DACPortOutputDec(OutputMode::MRFA_AMPLIFIER_PARAM_2);
                            }
                            break;
                        case OutputMode::RF_STATE_IMPEDANCE_CHECKING:
                            if(sysParm->Impedance_check_count < 2 )
                            {
                                if(div_level_state == true)
                                {
                                    div_level_state = false;
                                    mRfDeviceFactors->CheckImpedace_DAC = CHECKIMPEDANCE_DAC_MIN; //Impedance check DAC MIN
                                }
                            }
                            else if(sysParm->Impedance_check_count < 9 )
                            {
                                mRfDeviceFactors->CheckImpedace_DAC += 200; // max 8000
                            }
                            else
                            {
                                if(mRfDeviceFactors->rfImpedanceMeasured != 9990)
                                {
                                    //mRfDeviceFactors->rfImpedanceMeasured_last = mRfDeviceFactors->rfImpedanceMeasured;
                                    if(sysParm->mRfDeviceFactors->watt_select == WATT_7)
                                    {
                                        mRfDeviceFactors->rfDACTableMeasured_last = mRfDeviceFactors->rfDACTableMeasured7;
                                    }
                                    else if(sysParm->mRfDeviceFactors->watt_select == WATT_15)
                                    {
                                        mRfDeviceFactors->rfDACTableMeasured_last = mRfDeviceFactors->rfDACTableMeasured15;
                                    }
                                    else if(sysParm->mRfDeviceFactors->watt_select == WATT_25)
                                    {
                                        mRfDeviceFactors->rfDACTableMeasured_last = mRfDeviceFactors->rfDACTableMeasured25;
                                    }
                                    else if(sysParm->mRfDeviceFactors->watt_select == WATT_50)
                                    {
                                        mRfDeviceFactors->rfDACTableMeasured_last = mRfDeviceFactors->rfDACTableMeasured50;
                                    }
                                    RFStart();
                                }
                                else
                                {
                                    qDebug("Impedance Checking Fail");
                                    RF_Relay_State = false;
                                    //RFStop();

                                }
                                qDebug("v_division = %d, i_division = %d",
                                       mRfDeviceFactors->v_division_level,
                                       mRfDeviceFactors->i_division_level);
                                qDebug("count = %d", sysParm->Impedance_check_count);
                                break;
                            }
                            qDebug("Impedance Checking In count = %d", sysParm->Impedance_check_count);
                            //qDebug() << " CheckImpedance...";

                            outputMode->DACPortOutputSet(mRfDeviceFactors->CheckImpedace_DAC); //Impedance check DAC
                            sysParm->Impedance_check_count++;
                             //emit sig_rfStart();
                            break;

                    case OutputMode::RF_STATE_ACTIVE:
                            outputMode->MRFA_DoTestImpedanceState();
                            outputMode->RFPowerControl();
                            //mRfDeviceFactors->state = 0;
                        break;

                    }
            //RF output monitoring

            //calulator energy

            //warning

            //mRfDeviceFactors->state = 0;
            //mRfDeviceFactors->counting++;

    }
    else if(tmRFPowerActiveState == RF_ACTIVE_ON && RF_Relay_State == false)
    {
        RF_Relay_Count ++;
        if(RF_Relay_Count <= 1)
        {
            qDebug("DIsable DAC");
            outputMode->DACPortOutputSet(0);

        }
        else if(RF_Relay_Count > 2 && RF_Relay_Count <= 3)
        {
            qDebug("DIsable DCDC");
#ifndef _ONPC_
            gpioTools::DisableIsolated_DCDC();
#endif

        }
        else if(RF_Relay_Count > 4 && RF_Relay_Count <= 5)
        {
            qDebug("DIsable Relay");
#ifndef _ONPC_
            gpioTools::DisableTargetRelay_1st();
#endif
        }
        else if(RF_Relay_Count >= 6)
        {
#ifndef _ONPC_
            gpioTools::DisableTargetRelay_2nd();
#endif
            qDebug("RF Stop Function In");
            RF_Relay_Count = 0;
            RFStop();
            return ;
        }
        qDebug("State : RF_OFF, RF_Relay_Count = %d",RF_Relay_Count);
    }
    else if(tmRFPowerActiveState == RF_ACTIVE_OFF && RF_Relay_State == true)
    {

        RF_Relay_Count ++;
        if(RF_Relay_Count <= 1)
        {
            qDebug("Enable DCDC");
#ifndef _ONPC_
            gpioTools::EnableIsolated_DCDC();
            //outputMode->DACPortOutputSet(0);
#endif

        }
        else if(RF_Relay_Count > 39 && RF_Relay_Count <= 40)
        {
            qDebug("Enable Relay");
#ifndef _ONPC_
            gpioTools::EnableTargetRelay_2nd();
#endif
        }
        else if(RF_Relay_Count > 40 && RF_Relay_Count <= 41)
        {
#ifndef _ONPC_
            gpioTools::EnableTargetRelay_1st();
#endif
        }
        else if(RF_Relay_Count >= 43)
        {
            qDebug("Impedance_Check Function In");
            RF_Relay_Count = 0;
            Impedance_Check_Start();
            return ;
        }
        qDebug("State : RF_ON, RF_Relay_Count = %d",RF_Relay_Count);
    }
    if(sysParm->isDoTestforCal == true)
    {
#ifndef _ONPC_
        gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);
#endif

    }
    /*
     *  End call function.
     */
    sysParm->tmElapsedTime = elapsedTime.elapsed()/1000;
    sysParm->tmElapsedTime_count = elapsedTime.elapsed()/10; //10ms count

#if __INTERVAL_TEST__
    counter++;
#endif

    waitTime = __INTERVAL_MONITORING__ - _mTimer.elapsed();    // 30ms 32 - 34 per 1 second to the pc
    //waitTime = outputMode->Interbal_Monitoring - _mTimer.elapsed();
    //qDebug("%d",waitTime);
    if(waitTime >= 0)
    {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli> (waitTime));
    }
     //qDebug("%d",_countTimer.elapsed()); //outputMode->Interbal_Monitoring = 9  ----  9~10ms


}

void OutputThread::slot_dispCounter()
{
    qDebug("OutputThread run %d per 1seconds, waittime = %lf", counter, waitTime);
    counter = 0;
}


void OutputThread::SetOutputMode(OutputMode::RF_Output_Mode_en mode)
{
    if(tmRFPowerActiveState != RF_ACTIVE_ON)
    {

        if(mode == OutputMode::RF_DEVICE_OP_MODE_SIZE)
        {
            mode = outputMode->getMode();
            if(mode == OutputMode::RF_DEVICE_OP_MODE_CONSTANT)
            {
                mode = OutputMode::RF_DEVICE_OP_MODE_PULSE;
            }
            else if(mode == OutputMode::RF_DEVICE_OP_MODE_PULSE)
            {
                mode = OutputMode::RF_DEVICE_OP_MODE_CONSTANT;
            }
        }
        mRfDeviceFactors->mode_state = mode;
        outputMode = vModes.at(mode);
        qDebug("outputMode->getMode()= %d",outputMode->getMode());
        //emit sig_modeChange(mode);
    }
}

OutputMode::RF_Output_Mode_en OutputThread::GetOutputMode()
{
    return outputMode->getMode();
}

const int8* OutputThread::GetOutputModeTitle()
{
    return outputMode->getModeTitle();
}
void OutputThread::RF_Relay_ON()
{
    tmRFPowerActiveState = RF_ACTIVE_OFF;
    RF_Relay_State = true;
    qDebug("RF_Relay_ON");
}
void OutputThread::RF_Relay_OFF()
{
    RF_Relay_State = false;
    qDebug("RF_Relay_OFF");
}

void OutputThread::Impedance_Check_Start()
{
#if 1
#ifndef _ONPC_
        //EnableAmplifierPowerOutput();
#endif

#endif
        //emit sig_Graph_List_Clear();
        elapsedTime.restart();
        sysParm->tmElapsedTime = 0;
        sysParm->tmElapsedTime_count = 0;
        mRfDeviceFactors->counting = 0;
        sysParm->Impedance_check_count = 0;
        div_level_state = true;

        //mRfDeviceFactors->counting = 0;
        //mRfDeviceFactors->state=0;
        setPaused(true);
        tmRFPowerActiveState = RF_ACTIVE_ON;
        mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_IMPEDANCE_CHECKING;
        setPaused(false);
}

void OutputThread::RFStart()
{
    qDebug("GetMode = %d",outputMode->getMode());
    qDebug("outputTread_RF_Start");

    elapsedTime.restart();
    sysParm->tmElapsedTime = 0;
    sysParm->tmElapsedTime_count = 0;
    sysParm->Impedance_pulse_count = 0;
    sysParm->pulse_count = 0;
    mRfDeviceFactors->counting = 0;

    //mRfDeviceFactors->state= 1;

    //mRfDeviceFactors->tmTotalJ = 0;

    mRfDeviceFactors->tmRollOff = false;
    mRfDeviceFactors->isStepwise = false;
    mRfDeviceFactors->tmTooHighImpedanceDelayTime = 0;
    mRfDeviceFactors->tmTooHighTemperatureDelayTime = 0;
    mRfDeviceFactors->tmTooHighTempeatureFlag = false;
    mRfDeviceFactors->tmTooHighImpedanceFlag = false;


    setPaused(true);
    outputMode->RFStart();
    //tmRFPowerActiveState = RF_ACTIVE_ON;
    mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_ACTIVE;
    setPaused(false);

    //sysParm->isDoTestforCal = true;

    emit sig_rfStarted();
}

void OutputThread::RFStop()
{
    qDebug("outputTread_RF_Stop");
    outputMode->DACPortOutputDec(0);
    mRfDeviceFactors->CheckImpedace_DAC = CHECKIMPEDANCE_DAC_MIN;
#ifndef _ONPC_
    //DisableAmplifierPowerOutput();
#endif

    tmRFPowerActiveState = RF_ACTIVE_OFF;

    setPaused(true);
    outputMode->RFStop();
    mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_STOP;
    mRfDeviceFactors->tmDACValueToSetSave = 0;
    setPaused(false);
    emit sig_rfStopped();
}


bool OutputThread::GetRFPowerActiveState() const
{
    return tmRFPowerActiveState;
}

void OutputThread::SetRFPowerActiveState(bool state)
{
    tmRFPowerActiveState = state;
}

void OutputThread::MRFA_InitRFADeviceState()
{

}

void OutputThread::MRFA_DeInitRFADeviceState()
{

}

void OutputThread::CheckImpedanceState()
{
    //shot
    if(mRfDeviceFactors->tmReadADCCurrent > 50000)
    {
        qDebug("Short");
        RFStop();
        emit sig_overImpedance();
    }
    //low
#if 0
    if((mRfDeviceFactors->rfPowerLevel != 0 && mRfDeviceFactors->rfImpedanceMeasured <= 100) || //mRfDeviceFactors->rfImpedanceMeasured <= 10
      (mRfDeviceFactors->rfPowerLevel != 0 && mRfDeviceFactors->rfCurrentMeasured/10 >= 141))
    {
         mRfDeviceFactors->tmTooHighImpedanceFlag = true;

              if(mRfDeviceFactors->tmTooHighImpedanceDelayTime > 0) //mRfDeviceFactors->tmTooHighImpedanceDelayTime > 0 && sysParm->tmCutoff > 2
              {
                   qDebug("Impedance Low Currunt over");
                   RFStop();
                   emit sig_lowImpedance();
              }

    }

    else if(mRfDeviceFactors->rfPowerLevel != 0 &&
        mRfDeviceFactors->rfImpedanceMeasured <= 100 &&
        mRfDeviceFactors->rfCurrentMeasured/10 > mRfDeviceFactors->rfVoltageMeasured/10)
    {
         mRfDeviceFactors->tmTooHighImpedanceFlag = true;
            if(mRfDeviceFactors->tmTooHighImpedanceDelayTime > 0)
            {
                qDebug("Impedance Low");
                RFStop();
                emit sig_lowImpedance();
            }
    }

    else
    {
        mRfDeviceFactors->tmTooHighImpedanceFlag = false;
       // mRfDeviceFactors->tmTooHighImpedanceDelayTime = 0;
    }
#endif
}
void OutputThread::CheckOverTemperatureState()
{

}

void OutputThread::CheckSafetyState()
{

}

//void OutputThread::timerEvent(QTimerEvent *event)
//{
//    sysParm->tmElapsedTime = rfTimer.elapsed() / 1000;
//}
