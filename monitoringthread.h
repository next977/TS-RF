#ifndef MONITORINGTHREAD_H
#define MONITORINGTHREAD_H

#include <QDebug>
#include <QThread>
#include "gpiotools.h"
#include "sysparm.h"
#include "securedic.h"
#include "usbotg.h"

class MonitoringThread : public QThread
{
    Q_OBJECT

public:
    explicit MonitoringThread(QObject *parent = nullptr, SysParm *sysParm = nullptr);
    SysParm *sysParm;
private:

    LED_Color color_state;
    LED_Point point_state;

    securedic *SecuredIC;
    usbotg *USBOTG;

    double waitTime;
    bool LED_State;
    bool LED_Brightness_State;


    int32 LED_Count;
    int32 LED_Brightness_Add;
    int32 LED_Brightness_Sub;
    int32 LED_Brightness_Count;

    uint32 Temp_Count;
    uint32 SIC_Count;
    uint32 CQM_Count;
    uint32 LED_Control_Count;

    void run();

    void SetConnect();
    void Open_Device();
    void SetDefault_value();
    void SetDefault_function();
    void Set_Front_RGBLED();
    void Set_CQM();
    void Read_CQM();
    double CQM_calculate(uint32 CQM_Data);
    void State_Change_RGBLED();
    void Control_RGBLED();
    void Display_Front_RGBLED(LED_Color color, LED_Point point, int32 brightness);
    void SIC_Control();
    void SIC_Control_Function();
    void Temp_Fan_Control();
    void Temp_Fan_Function();

signals:
    void sig_SecuredIC_Start();
    void sig_Set_Needle(State_Needle state, Length_Needle length, Angle_Needle angle );
    void sig_State_check();
};

#endif // MONITORINGTHREAD_H
