#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFontDatabase>
#include <QFont>
#include <QTimer>
#include <QElapsedTimer>
#include <QPushButton>
#include <QLabel>
#include "outputthread.h"
#include "monitoringtask.h"
#include "starmedlogo.h"
#include "sysmessage.h"
#include "setup.h"
#include "gpiotools.h"
#include "sysparm.h"
#include "gpiobutton.h"
#include "audiooutput.h"
#include "file.h"
#include "monitoringthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void setIDstarmed(int i) {id_starmed = i; }
    bool eventFilter(QObject *target, QEvent *event);
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slot_Main_ScreenIN();
    void slot_Main_screenIn();
    void slot_btnUDPressed(BTNS btn);
    void slot_btnShortPressed(BTNS state);
    void slot_btnLongPressed(BTNS state);
    void slot_rfStarted();
    void slot_rfStopped();
    void slot_Set_Mode(Mode_RF mode);
    void slot_Set_Needle(State_Needle state, Length_Needle length, Angle_Needle angle );
    void slot_State_check();

private:
    Ui::MainWindow *ui;
    MonitoringTask *monitoringTask;
    OutputThread *outputThread;
    starmedlogo *Starmedlogo_Screen;
    setup *Setup_Screen;
    sysmessage *sysmessage_Screen;
    SysParm *sysParm;
    GPIOButton *pGButton;
    AudioOutput *Audio_Output;
    File_RW *file_rw;

    MonitoringThread *monitoringthread;

    QTimer *Button_Timer;
    QTimer *Display_Timer;

    QPushButton *PressedButton;
    QLabel *Label;

    int Button_Counting;
    int id_starmed;
    bool IsPressed;

    void Setting_RTCVolume();
    void SetEventFilter();
    void SetConnect();
    void SetDefault_function();
    void SetDefault_value();
    void SetImage();
    void SetFont();

    void Set_State(State_EN state);
    void Set_State_Needle(State_Needle needle );
    void Set_Angle_Needle( Angle_Needle angle );
    void Set_Length_Needle( Length_Needle length );
    void Set_Pad(State_Pad pad );

    void Open_Device();
    void UI_Hide();
    void UI_Show();
    void UI_Enable();
    void UI_Disable();

    void LCD_Display();

#ifdef __TSRF_TEST__
    void Test_Display();
#endif


private slots:
    void slot_Button_Loop();
    void slot_Display();

signals:
    void sig_Setup_Screen();
    void sig_btnShortPressed(BTNS state);
    void sig_btnLongPressed(BTNS state);
    void sig_alarm_code(Alarm_Code code);
    void sig_error_code(Error_Code code);
    void sig_Set_Mode(Mode_RF mode);

};

#endif // MAINWINDOW_H
