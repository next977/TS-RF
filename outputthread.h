#ifndef OUTPUTTHREAD_H
#define OUTPUTTHREAD_H

#include <QObject>
#include "workerthread.h"
#include "outputmode.h"

#include <QTime>
#include <QTimer>
#include <QBasicTimer>
#include "monitoringtask.h"

using namespace std;
class SysParm;


class OutputThread : public QObject, public WorkerThread
{
    Q_OBJECT
public:
    explicit OutputThread(QObject *parent = nullptr, SysParm *sysParm = nullptr);

    ~OutputThread();

signals:
    void sig_modeChange(OutputMode::RF_Output_Mode_en);
    //void sig_rfStart();
    void sig_rfStarted();
    //void sig_rfStop();
    void sig_rfStopped();
    void sig_overTemperature();
    void sig_overImpedance();
    void sig_lowImpedance();
    //void sig_SIC_Start();
public slots:
    void slot_dispCounter();

private:
    OutputMode *outputMode;
    MonitoringTask *monitoringTask;
    QElapsedTimer elapsedTime;

    vector<OutputMode*> vModes;

    //QElapsedTimer elapsedTime;
    bool tmRFPowerActiveState;
    void loop();
    clock_t begin;
    //uint32 SIC_Count;

public:
    SysParm *sysParm;
    RF_DEVICE_FACTORS *mRfDeviceFactors;

    //QBasicTimer btimer;
    bool RFStop_State;
    void SetOutputMode(OutputMode::RF_Output_Mode_en = OutputMode::RF_DEVICE_OP_MODE_SIZE);

    OutputMode::RF_Output_Mode_en GetOutputMode();

    const int8* GetOutputModeTitle();
    void Impedance_Check_Start();


    void RFStart();
    void RFStop();
    bool GetRFPowerActiveState() const;
    void SetRFPowerActiveState(bool state);
    void MRFA_InitRFADeviceState();
    void MRFA_DeInitRFADeviceState();
    //void DACPortOutputInc(U16, bool pass=true);
    //void DACPortOutputDec(U16, bool pass=true);
    //void DACPortOutputSet(U16);
    void RF_Relay_ON();
    void RF_Relay_OFF();
    void CheckImpedanceState();
    void CheckOverTemperatureState();
    void CheckSafetyState();
    void InitThread(SysParm *sysParm);


    //void timerEvent(QTimerEvent *event);


//for test
private:
    int counter;
    uint32 Impedance_check_count;
    uint32 RF_Relay_Count;
    //int PWM_count;
    //bool PWM_Count_State;
    bool RF_Relay_State;
    bool div_level_state;
    double waitTime;
    QTimer *checkpersec;
   // bool Check_state = false;

};
#endif // OUTPUTTHREAD_H
