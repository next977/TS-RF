#ifndef MONITORINGTASK_H
#define MONITORINGTASK_H

#include <QObject>
#include <QElapsedTimer>
#include <QDebug>
#include "sysparm.h"

class QTimer;

#define MEDIAN_ORDER 3
#define MEDIAN_DEVICE_ORDER 3

class MonitoringTask : public QObject
{
    Q_OBJECT
public:
    explicit MonitoringTask(QObject *_parent, SysParm *_sysParm);
    ~MonitoringTask();
    //void InitTask(SysParm *_sysParm);

   void RunTimer();
	
signals:

public slots:
    void slot_dispCounter();
    void run();
    void GetAmplifierParameters();

private:
    uint16 IOADCSPIPort(int32 n_cs, int32 channel);
    uint16 MedianFilter(int32 deviceid, int32 index, uint16 data);
    void MRFA_TranslateCalculate();
    void MRFA_TranslateRFTable();
private:
    SysParm *sysParm;
    QTimer *timer;

    uint16 tm_median_voltage[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];
    uint16 tm_median_voltage_sort[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];

    uint16 tm_median_current[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];
    uint16 tm_median_current_sort[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];

    uint16 tm_median_tempA[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];
    uint16 tm_median_temp_sortA[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];

    uint16 tm_median_tempB[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];
    uint16 tm_median_temp_sortB[MEDIAN_DEVICE_ORDER][MEDIAN_ORDER];


//for test
private:
    int counter;
    uint16 adc_temp;
    uint16 adc_voltage;
    uint16 adc_current;

};

#endif // MONITORINGTASK_H
