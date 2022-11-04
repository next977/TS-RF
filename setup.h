#ifndef SETUP_H
#define SETUP_H

#include <QWidget>
#include <QFontDatabase>
#include <QFont>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "sysparm.h"
#include "gpiotools.h"
#include "calibration.h"

namespace Ui {
class setup;
}

class setup : public QWidget
{
    Q_OBJECT

public:
    SysParm *sysParm;
    void setIDstarmed(int i) {id_starmed = i; }
    bool eventFilter(QObject *target, QEvent *event);

    explicit setup(QWidget *parent = nullptr , SysParm *sysParm=nullptr);
    ~setup();

public slots:
    void slot_Setup_ScreenIN();
    void slot_Setup_Screenin();


private:
    Ui::setup *ui;
    QTimer *ButtonTimer;
    QTimer *DisplayTimer;
    calibration *Calibration_Screen;

    int Button_Counting;
    int id_starmed;
    bool IsPressed;
    bool LED_State;
    bool State_Mode;
    bool State_Language;
    int32 LED_Count;

    QPushButton *PressedButton;
    QLabel *Label;

    LED_Color color_state;
    LED_Point point_state;


    void Display_Front_RGBLED(LED_Color color, LED_Point point, int32 brightness);
    void LCD_Display();
    void Control_RGBLED();
    void Choose_Click_state(Click_State state);

    void SetDefault_function();
    void SetDefault_value();
    void SetFont();
    void SetEventFilter();
    void SetConnect();
    void SetUI_Color();
    void SetImage();
    void Set_Mode(Mode_RF mode);
    void Set_Language();

    void UI_Show();
    void UI_Hide();


private slots:
    void slot_Button_Loop();
    void slot_Display();

signals:
    void sig_Main_Screen();
    void sig_btnUDPressed(BTNS state);
    void sig_ScreenIn_Calibration(Call_State State);
    void sig_Set_Mode(Mode_RF mode);
};

#endif // SETUP_H
