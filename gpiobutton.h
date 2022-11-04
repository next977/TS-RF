#ifndef GPIOBUTTON_H
#define GPIOBUTTON_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "sysparm.h"
#include "gpiotools.h"


class GPIOButton : public QObject
{
    Q_OBJECT

    typedef struct BUTTONS {
        uint32 gpio;
        uint32 pin;
        unsigned long debounce;
        uint32 prevState;
        uint32 currentState;
        bool islongPress;
        bool stopState;
    }BUTTON;


public:
    explicit GPIOButton(QObject *parent = 0);
    ~GPIOButton();
    enum BTNS{BTN_RFSWITCH=0, BTN_FOOTSWITCH, BTN_MAX};
    enum BTN_STATE
    {
        BTN_STATE_RFREADY=0,
        BTN_STATE_RFSTART,
        BTN_STATE_RFSTOP
    }Button_state;
    State_EN State_system;

    const uint32 BTN_NOT_PRESSED = gpioTools::GPIO_HIGH;
    const uint32 BTN_PRESSED = gpioTools::GPIO_LOW;

    void readBtnStatus(uint32 id);

private:
    QTimer timer;
    BUTTON buttons[BTN_MAX];
    QElapsedTimer keyElapsed;
    //bool Button_state = false;
signals:
    void sig_btnShortPressed(BTNS);
    void sig_btnLongPressed(BTNS);

public slots:
    void loop();

};

#endif // GPIOBUTTON_H
