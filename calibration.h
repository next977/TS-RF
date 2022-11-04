#ifndef CALIBRATION_H
#define CALIBRATION_H
#include "global.h"
#include "sysparm.h"
#include "gpiotools.h"
#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QSocketNotifier>
#include <QLabel>
#include <QLCDNumber>
class SysParm;
namespace Ui {
class calibration;
}

class calibration : public QWidget
{
    Q_OBJECT


public:
    SysParm *sysParm;
    uint16 data_in_index;
    void setIDstarmed(int i) {id_starmed = i; }
    bool eventFilter(QObject *target, QEvent *event);
    explicit calibration(QWidget *parent = nullptr, SysParm *sysParm = nullptr);
    ~calibration();

public slots:
    void slot_ScreenIn_Calibration(Call_State State);

private:
    Ui::calibration *ui;

    QTimer *DisplayTimer;
    QTimer *Button_Timer;
    QPushButton *PressedButton;
    QSocketNotifier *notRsRead;
    QLCDNumber *LCDNumber;
    QLabel *Label;
    int id_starmed;
    int Button_Counting;
    bool IsPressed;
    bool IsUSBstate;
    void SetFont();
    void Calibration_connect();
    void Choose_Click_state(Click_State state);
    void installFilter();
    void Limited_Button();
    void UI_Hide();
    void UI_Show();
    QPalette Ui_SetColor(QPalette::ColorRole , Qt::GlobalColor , QPalette::ColorRole , Qt::GlobalColor );
private slots:
    void slot_display();
    void slot_Button_Loop();
    //void slot_calibration_in();
    void slot_btnReleasedCal(int keyEvent);
    void slot_btnDirReleasedCal(int keyEvent);
    void slot_Read_Event();

signals:
    //void sig_setting();
    void sig_ScreenIn_Setup();
   // void sig_calibration();
    void sig_btnDirReleasedCal(int);
    void sig_btnReleasedCal(int);
    void sig_ScreenIn_Engineer(Call_State);
};

#endif // CALIBRATION_H
