#include "gpiobutton.h"

GPIOButton::GPIOButton(QObject *parent) : QObject(parent)
{

    buttons[BTN_RFSWITCH].gpio = gpioTools::GPIO_6;
    buttons[BTN_RFSWITCH].pin = 3;
    buttons[BTN_RFSWITCH].debounce = 10;
    buttons[BTN_RFSWITCH].prevState = BTN_PRESSED;
    buttons[BTN_RFSWITCH].islongPress = false;

    buttons[BTN_FOOTSWITCH].gpio = gpioTools::GPIO_5;
    buttons[BTN_FOOTSWITCH].pin = 20;
    buttons[BTN_FOOTSWITCH].debounce = 10;
    buttons[BTN_FOOTSWITCH].prevState = BTN_NOT_PRESSED;
    buttons[BTN_FOOTSWITCH].islongPress = false;

    connect(&timer, SIGNAL(timeout()), this, SLOT(loop()));
    timer.setInterval(10);
    timer.start();
    keyElapsed.start();
    Button_state = BTN_STATE_RFSTART;
}

GPIOButton::~GPIOButton()
{
}

void GPIOButton::readBtnStatus(uint32 id)
{
#ifdef Button_Released
    buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin); //struct
    if(buttons[id].currentState != buttons[id].prevState)
    {
        mdelay(buttons[id].debounce);
        buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin);
        if(buttons[id].currentState == BTN_PRESSED)
        {
            keyElapsed.restart();
            qDebug("id = %d , Pressd" , id);

        }

        else if(buttons[id].currentState == BTN_NOT_PRESSED)
        {
            if((keyElapsed.elapsed() > 100) && !(keyElapsed.elapsed() >= 500))
            {
                emit sig_btnReleased(id);
                qDebug("id = %d , sig_Released" , id);
            }
            else if (keyElapsed.elapsed() >= 500)
            {
                emit sig_btnLongPressed(id);
                qDebug("id = %d , sig_LongPressed" , id);

            }
        }
       buttons[id].prevState = buttons[id].currentState;
    }

#endif
    buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin); //struct
    if(buttons[id].currentState != buttons[id].prevState)
    {
        mdelay(buttons[id].debounce);
        buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin);
        if(id == BTN_RFSWITCH)
        {
            if(buttons[id].currentState == BTN_NOT_PRESSED) //Push
            {

                keyElapsed.restart();
                if(Button_state == BTN_STATE_RFREADY && buttons[id].islongPress == true)
                {
                    if(State_system == STATE_SYSTEM_ON)
                    {
                        Button_state = BTN_STATE_RFSTOP;
                    }
                    else if(State_system == STATE_SYSTEM_READY)
                    {
                        Button_state = BTN_STATE_RFSTART;
                        buttons[id].islongPress = false;
                    }
                }
                else if(Button_state == BTN_STATE_RFSTART && buttons[id].islongPress == true)
                {
                    buttons[id].islongPress = false;
                }

            }
            else if(buttons[id].currentState == BTN_PRESSED) //Realease
            {
                /*
                if(Button_state == BTN_STATE_RFREADY && buttons[id].islongPress == false)
                {
                    Button_state = BTN_STATE_RFSTART;
                }
                else
                {
                    buttons[id].islongPress = true;
                }
                */

                buttons[id].islongPress = true;
            }
        }
        else if(id == BTN_FOOTSWITCH)
        {
            if(buttons[id].currentState == BTN_PRESSED)
            {

                keyElapsed.restart();
                Button_state = BTN_STATE_RFSTART;
                buttons[id].islongPress = false;
                //Button_state = BTN_STATE_RFSTART;
               //emit sig_btnReleased(id);
            }
            else if(buttons[id].currentState == BTN_NOT_PRESSED)
            {
                if(State_system == STATE_SYSTEM_ON)
                {
                    Button_state = BTN_STATE_RFSTOP;
                }
            }
        }

       buttons[id].prevState = buttons[id].currentState;
    }
    if(Button_state == BTN_STATE_RFSTART)
    {
        /*
        if ((keyElapsed.elapsed() >= (RFSTARTBUTTON_TIME_MS-(DCDC_RELAY_TIME_MS+20)))&&(keyElapsed.elapsed() <= (RFSTARTBUTTON_TIME_MS-DCDC_RELAY_TIME_MS)+100)&&
                ((buttons[BTN_FOOTSWITCH].currentState == BTN_PRESSED)||((buttons[BTN_RFSTARTSTOP].currentState == BTN_NOT_PRESSED)))) //380ms
        {
        */
        if (id == BTN_RFSWITCH && buttons[BTN_RFSWITCH].islongPress == false &&
            buttons[BTN_RFSWITCH].currentState == BTN_NOT_PRESSED) //BTN_RFSWITCH Start
        {
            Button_state = BTN_STATE_RFREADY;
            emit sig_btnLongPressed(BTN_RFSWITCH);
            qDebug("RF Switch Start");
        }
        else if(id == BTN_FOOTSWITCH &&
                buttons[BTN_FOOTSWITCH].currentState == BTN_PRESSED &&
                buttons[id].islongPress == false) //BTN_FOOTSWITCH Start
        {
            Button_state = BTN_STATE_RFREADY;
            buttons[id].islongPress = true;
            emit sig_btnLongPressed(BTN_FOOTSWITCH);
            qDebug("FootSwtich Start");

        }
    }
    else if(Button_state == BTN_STATE_RFSTOP)
    {

        if (id == BTN_RFSWITCH && buttons[BTN_RFSWITCH].currentState == BTN_PRESSED &&
               buttons[BTN_RFSWITCH].islongPress == true ) //BTN_RFSWITCH Start
        {
            Button_state = BTN_STATE_RFSTART;
            emit sig_btnShortPressed(BTN_RFSWITCH);
            qDebug("RF Switch Stop");
        }
        else if (id == BTN_FOOTSWITCH &&
                 buttons[BTN_FOOTSWITCH].currentState == BTN_NOT_PRESSED &&
                 buttons[id].islongPress == true )
        {
            Button_state = BTN_STATE_RFSTOP;
            buttons[id].islongPress = false;
            emit sig_btnShortPressed(BTN_FOOTSWITCH);
            qDebug("FootSwitch Stop");
        }
    }
}

void GPIOButton::loop()
{
    for(uint32 i ; i <BTN_MAX ; i ++ )
    {
        readBtnStatus(i);

    }
}
