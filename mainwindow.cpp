#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFontDatabase>
#include <QDebug>
#include <QFont>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifndef _ONPC_
    gpioTools::gpio_init();
    gpioTools::spi_init();
    Open_Device();
#endif
    this->setIDstarmed(QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf"));
    //Setgif();
    //ui->image_screen->installEventFilter(this);
    sysParm = new SysParm();
    pGButton = new GPIOButton();
    Starmedlogo_Screen = new starmedlogo(this);
    sysmessage_Screen = new sysmessage(this);
    Audio_Output = new AudioOutput(this);
    Setup_Screen = new setup(this, sysParm);
    file_rw = new File_RW(this,sysParm);
    Display_Timer = new QTimer(this);
    Button_Timer = new QTimer(this);
    outputThread = new OutputThread(this, sysParm);
    monitoringthread = new MonitoringThread(this, sysParm);
    monitoringthread->start();
    outputThread->start();
    Setup_Screen->close();
    sysmessage_Screen->close();
    SetDefault_value();
    SetDefault_function();
#ifndef _ONPC_
    gpioTools::DisableAmplifierPowerOutput();
#endif
#if 1
    sysParm->mRfDeviceFactors->Date_year = 22;
    sysParm->mRfDeviceFactors->Date_month = 1;
    sysParm->mRfDeviceFactors->Date_day  = 10;
    sysParm->mRfDeviceFactors->Time_hour = 14;
    sysParm->mRfDeviceFactors->Time_min = 53;
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::Open_Device()
{
#ifndef _ONPC_
    gpioTools::uart_init();
    gpioTools::Volume_Open();
    //gpioTools::RTC_Open();
#endif
}

void MainWindow::SetDefault_value()
{
    IsPressed = false;
    sysParm->mRfDeviceFactors->state_needle = STATE_NEEDLE_OFF;
    sysParm->mRfDeviceFactors->state_pad = STATE_PAD_OFF;
    sysParm->mRfDeviceFactors->state_system = STATE_SYSTEM_STANDBY;
    sysParm->mRfDeviceFactors->current_state_system = STATE_SYSTEM_STANDBY;
    ///////////////////sig_Set_Needle///////////////////////////
    qRegisterMetaType<State_Needle>("State_Needle");
    qRegisterMetaType<Length_Needle>("Length_Needle");
    qRegisterMetaType<Angle_Needle>("Angle_Needle");
    ///////////////////////////////////////////////////////////
}

void MainWindow::SetDefault_function()
{
    SetFont();
    SetConnect();
    SetEventFilter();
    SetImage();
#ifndef _ONPC_
    Audio_Output->playSound(AudioOutput::SND_BOOTING);
    Setting_RTCVolume();
#endif
    UI_Hide();
    ui->needle_angle_label->hide();
    ui->needle_length_label->hide();
    ui->needle_angle_symbol_label->hide();
    ui->needle_length_symbol_label->hide();
    emit sig_Set_Mode(Mode_RF::MODE_RF_PULSE);
}
void MainWindow::SetConnect()
{
    connect(Button_Timer, SIGNAL(timeout()), this, SLOT(slot_Button_Loop()));
    connect(Display_Timer, SIGNAL(timeout()), this, SLOT(slot_Display()));
    connect(Starmedlogo_Screen, SIGNAL(sig_Main_Screen()), this, SLOT(slot_Main_ScreenIN()));
    connect(Setup_Screen, SIGNAL(sig_Main_Screen()), this, SLOT(slot_Main_screenIn()));
    connect(Setup_Screen, SIGNAL(sig_btnUDPressed(BTNS)), this, SLOT(slot_btnUDPressed(BTNS)));
    connect(Setup_Screen, SIGNAL(sig_Set_Mode(Mode_RF)), this, SLOT(slot_Set_Mode(Mode_RF)));
    connect(sysmessage_Screen, SIGNAL(sig_Main_Screen()), this, SLOT(slot_Main_screenIn()));
    //<----------------------------------- Foot Switch ------------------------------------>
    connect(pGButton, SIGNAL(sig_btnShortPressed(BTNS)), this, SLOT(slot_btnShortPressed(BTNS)));
    connect(pGButton, SIGNAL(sig_btnLongPressed(BTNS)), this, SLOT(slot_btnLongPressed(BTNS)));
    //<------------------------------------------------------------------------------------>
    connect(outputThread, SIGNAL(sig_rfStarted()), this, SLOT(slot_rfStarted()));
    connect(outputThread, SIGNAL(sig_rfStopped()), this, SLOT(slot_rfStopped()));
    //connect(outputThread, SIGNAL(sig_SIC_Start()), SecuredIC, SLOT(slot_SecuredIC_Start()));
    //<-----------------------------------Sensor ------------------------------------------>
    connect(monitoringthread, SIGNAL(sig_Set_Needle(State_Needle, Length_Needle, Angle_Needle)), this, SLOT(slot_Set_Needle(State_Needle, Length_Needle, Angle_Needle)));
    connect(monitoringthread, SIGNAL(sig_State_check()), this, SLOT(slot_State_check()));
    //<------------------------------------------------------------------------------------>
    connect(this, SIGNAL(sig_btnShortPressed(BTNS)), this, SLOT(slot_btnShortPressed(BTNS)));
    connect(this, SIGNAL(sig_btnLongPressed(BTNS)), this, SLOT(slot_btnLongPressed(BTNS)));
    connect(this, SIGNAL(sig_Set_Mode(Mode_RF)), this, SLOT(slot_Set_Mode(Mode_RF)));
    connect(this, SIGNAL(sig_Setup_Screen()), Setup_Screen, SLOT(slot_Setup_ScreenIN()));
    connect(this, SIGNAL(sig_alarm_code(Alarm_Code)), sysmessage_Screen, SLOT(slot_alarm_code(Alarm_Code)));
    connect(this, SIGNAL(sig_error_code(Error_Code)), sysmessage_Screen, SLOT(slot_error_code(Error_Code)));
}

void MainWindow::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont state_title(family,50, QFont::Bold);
    QFont mode_title(family,25, QFont::Bold);
    QFont time_title(family,25, QFont::Bold);
    QFont test_title(family,15, QFont::Bold);
    QFont button(family,20, QFont::Bold);
    QFont needle(family,30, QFont::Bold);
    QFont symbol(family,20, QFont::Bold);
    QFont unit(family,50, QFont::Bold);
    QFont data(family,90, QFont::Bold);
    QFont mode_unit(family,20);
    QFont event(family,10, QFont::Bold);
#ifdef __TSRF_TEST__
    ui->V_Test_title_label->setFont(test_title);
    ui->I_Test_title_label->setFont(test_title);
    ui->DAC_Test_title_label->setFont(test_title);
    ui->R_Test_title_label->setFont(test_title);
    ui->CQM_Aver_Test_title_label->setFont(test_title);
    ui->V_Test_data_label->setFont(test_title);
    ui->I_Test_data_label->setFont(test_title);
    ui->DAC_Test_data_label->setFont(test_title);
    ui->R_Test_data_label->setFont(test_title);
    ui->CQM_Aver_Test_data_label->setFont(test_title);
    ui->CQM_Cal_Test_data_label->setFont(test_title);
    ui->CQM_Cal_Test_title_label->setFont(test_title);
#endif
    //label size
    ui->state_title_label->setFont(state_title);
    ui->mode_title_label->setFont(mode_title);
    ui->time_title_label->setFont(time_title);
    ui->time_unit_label->setFont(unit);
    ui->time_data_label->setFont(data);
    ui->mode_unit0_label->setFont(mode_unit);
    ui->mode_unit1_label->setFont(mode_unit);
    ui->needle_angle_label->setFont(needle);
    ui->needle_length_label->setFont(needle);
    ui->needle_angle_symbol_label->setFont(symbol);
    ui->needle_length_symbol_label->setFont(symbol);
    ui->alarm_label->setFont(event);
    ui->error_label->setFont(event);
    ui->state_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->state_title_label->setText("Standby");
    ui->time_title_label->setText("Time");
    ui->mode_title_label->setText("Pulse");
    ui->needle_angle_label->setText("50");
    ui->needle_angle_symbol_label->setText("º");
    ui->needle_length_label->setText("98");
    ui->needle_length_symbol_label->setText("cm");
    ui->state_title_label->setAlignment(Qt::AlignCenter);
    ui->time_title_label->setAlignment(Qt::AlignCenter);
    ui->time_data_label->setAlignment(Qt::AlignCenter);
    ui->mode_title_label->setAlignment(Qt::AlignCenter);
}


void MainWindow::SetImage()
{
    //button
    ui->pad_button_label->setPixmap(QPixmap(":/Images/pad_disable.png"));
    ui->needle_button_label->setPixmap(QPixmap(":/Images/electrode_disable.png"));
    ui->up_button_label->setPixmap(QPixmap(":/Images/button_up_enable.png"));
    ui->down_button_label->setPixmap(QPixmap(":/Images/button_down_enable.png"));
    //label
    ui->image_screen->setPixmap(QPixmap(":/Images/state_standby.png"));
    ui->setup_label->setPixmap(QPixmap(":/Images/button_setup_enable.png"));
    ui->time_tag_label->setPixmap(QPixmap(":/Images/time_tag_enable.png"));
    ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_tag_pulse.png"));
    ui->mode_symbol_label->setPixmap(QPixmap(":/Images/pulse.png"));
    //button
    ui->pad_button_label->setScaledContents(true);
    ui->needle_button_label->setScaledContents(true);
    ui->up_button_label->setScaledContents(true);
    ui->down_button_label->setScaledContents(true);
    //label
    ui->image_screen->setScaledContents(true);
    ui->setup_label->setScaledContents(true);
    ui->time_tag_label->setScaledContents(true);
    ui->mode_tag_label->setScaledContents(true);
    ui->mode_symbol_label->setScaledContents(true);
}

void MainWindow::SetEventFilter()
{
    ui->pad_button_label->installEventFilter(this);
    ui->needle_button_label->installEventFilter(this);
    ui->up_button_label->installEventFilter(this);
    ui->down_button_label->installEventFilter(this);
    ui->mode_tag_label->installEventFilter(this);
    ui->setup_label->installEventFilter(this);
    ui->alarm_label->installEventFilter(this);
    ui->error_label->installEventFilter(this);
    ui->mode_symbol_label->installEventFilter(this);
}
void MainWindow::slot_Set_Needle(State_Needle state, Length_Needle length, Angle_Needle angle )
{
    sysParm->mRfDeviceFactors->state_needle = state;
    switch(state)
    {
        case STATE_NEEDLE_ON:
            ui->needle_button_label->setPixmap(QPixmap(":/Images/electrode_enable.png"));
            switch(angle)
            {
                case ANGLE_NEEDLE_50:
                    ui->needle_angle_label->setText("50");
                    break;
                case ANGLE_NEEDLE_86:
                    ui->needle_angle_label->setText("86");
                    break;

                default :
                    break;
            }
            switch(length)
            {
                case LENGTH_NEEDLE_98:
                    ui->needle_length_label->setText("98");
                    break;
                case LENGTH_NEEDLE_89:
                    ui->needle_length_label->setText("89");
                    break;
                case LENGTH_NEEDLE_71:
                    ui->needle_length_label->setText("71");
                    break;
                default :
                    break;
            }
            ui->needle_angle_label->show();
            ui->needle_length_label->show();
            ui->needle_angle_symbol_label->show();
            ui->needle_length_symbol_label->show();
            break;

        case STATE_NEEDLE_OFF:
            ui->needle_button_label->setPixmap(QPixmap(":/Images/electrode_disable.png"));
            ui->needle_angle_label->hide();
            ui->needle_length_label->hide();
            ui->needle_angle_symbol_label->hide();
            ui->needle_length_symbol_label->hide();
            break;
        default :
            break;
    }

}
void MainWindow::slot_Set_Mode(Mode_RF mode)
{
    sysParm->mRfDeviceFactors->mode_rf = mode;
    //qDebug("mode_rf = %d",mode_rf);
    switch(mode)
    {
        case Mode_RF::MODE_RF_PULSE:
            outputThread->SetOutputMode(OutputMode::RF_DEVICE_OP_MODE_PULSE);
            ui->mode_title_label->setText("Pulse");
            ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_tag_enable_pulse.png"));
            ui->mode_symbol_label->setPixmap(QPixmap(":/Images/mode_tag_pulse.png"));
            break;
        case Mode_RF::MODE_RF_CONSTANT:
            outputThread->SetOutputMode(OutputMode::RF_DEVICE_OP_MODE_CONSTANT);
            ui->mode_title_label->setText("Constant");
            ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_tag_enable_constant.png"));
            ui->mode_symbol_label->setPixmap(QPixmap(":/Images/mode_tag_constant.png"));
            break;
        default :
            break;
    }
}

void MainWindow::Set_State(State_EN state)
{
    sysParm->mRfDeviceFactors->state_system = state;
    pGButton->State_system = state;
    switch(state)
    {
        case STATE_SYSTEM_STANDBY:
            ui->state_title_label->setText("Standby");
            ui->image_screen->setPixmap(QPixmap(":/Images/state_standby.png"));
            break;
        case STATE_SYSTEM_READY:
            ui->state_title_label->setText("Ready");
            ui->image_screen->setPixmap(QPixmap(":/Images/state_ready.jpg"));
            break;
        case STATE_SYSTEM_ON:
            ui->state_title_label->setText("On");
            ui->image_screen->setPixmap(QPixmap(":/Images/state_on.png"));
            break;
        case STATE_SCREEN_SETUP:
            break;

        default :
            break;
    }
}

void MainWindow::Set_Pad(State_Pad pad )
{
    sysParm->mRfDeviceFactors->state_pad = pad;
    switch(pad)
    {
        case STATE_PAD_ON:
            ui->pad_button_label->setPixmap(QPixmap(":/Images/pad_enable.png"));
            break;
        case STATE_PAD_OFF:
            ui->pad_button_label->setPixmap(QPixmap(":/Images/pad_disable.png"));
            break;
        default :
            break;
    }
}

void MainWindow::Setting_RTCVolume()
{
#ifndef _ONPC_
    gpioTools::MAX9768_Write(gpioTools::OM_FLITERLESS, sysParm->mRfDeviceFactors->Volume*7);
#endif
    qDebug() << "sysParm->mRfDeviceFactors->Volume = " <<sysParm->mRfDeviceFactors->Volume;
#ifndef _ONPC_
    QString Date_year, Date_month, Date_day, Time_hour, Time_min, Time_sec;
    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_PWR_MGMT, 0x1c); //0x10
    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_TRICKLE_REG, 0x00); //0x50
    Date_year.sprintf("%x", gpioTools::MAX31343_Read(gpioTools::RTC_REGISTERS_YEARS));
    Date_month.sprintf("%x", gpioTools::MAX31343_Read(gpioTools::RTC_REGISTERS_MONTHS));
    Date_day.sprintf("%x", gpioTools::MAX31343_Read(gpioTools::RTC_REGISTERS_DATES));
    Time_hour.sprintf("%x", gpioTools::MAX31343_Read(gpioTools::RTC_REGISTERS_HOURS));
    Time_min.sprintf("%x", gpioTools::MAX31343_Read(gpioTools::RTC_REGISTERS_MINUTES));
    Time_sec.sprintf("%x", gpioTools::MAX31343_Read(gpioTools::RTC_REGISTERS_SECONDS));
    sysParm->mRfDeviceFactors->Date_year = Date_year.toUInt();
    sysParm->mRfDeviceFactors->Date_month = Date_month.toUInt();
    sysParm->mRfDeviceFactors->Date_day  = Date_day.toUInt();
    sysParm->mRfDeviceFactors->Time_hour = Time_hour.toUInt();
    sysParm->mRfDeviceFactors->Time_min = Time_min.toUInt();
    sysParm->mRfDeviceFactors->Time_sec = Time_sec.toUInt();
    qDebug()<<"Date_year" << Date_year ;
    qDebug()<<"Date_month" << Date_month ;
    qDebug()<<"Date_day" << Date_day ;
    qDebug()<<"Time_hour" << Time_hour;
    qDebug()<<"Time_min" << Time_min ;
    qDebug()<<"Time_sec" << Time_sec ;
    QString time_date = "20"+Date_year+'-'+Date_month+'-'+Date_day+' '+Time_hour+':'+Time_min+':'+Time_sec;
    qDebug() << "timer_date="<<time_date;
    QString time_format = "yyyy-MM-dd HH:mm:ss";
    QDateTime datetimer = QDateTime::fromString(time_date, time_format);
    qDebug() << "datetimer = "<<datetimer;
    qDebug() << "datetimer toString()= "<<datetimer.toString();
    qDebug() << "datetimer toUTC() = "<<datetimer.toUTC().toString();
#endif
}

void MainWindow::UI_Hide()
{
    ui->image_screen->hide();
    ui->up_button_label->hide();
    ui->down_button_label->hide();
    ui->mode_tag_label->hide();
    ui->mode_title_label->hide();
    ui->mode_symbol_label->hide();
    ui->mode_unit0_label->hide();
    ui->mode_unit1_label->hide();
    ui->needle_button_label->hide();
    ui->pad_button_label->hide();
    ui->setup_label->hide();
    ui->state_title_label->hide();
    ui->time_data_label->hide();
    ui->time_tag_label->hide();
    ui->time_title_label->hide();
    ui->time_unit_label->hide();
    ui->alarm_label->hide();
    ui->error_label->hide();
#ifdef __TSRF_TEST__
    ui->V_Test_title_label->hide();
    ui->I_Test_title_label->hide();
    ui->DAC_Test_title_label->hide();
    ui->R_Test_title_label->hide();
    ui->V_Test_data_label->hide();
    ui->I_Test_data_label->hide();
    ui->DAC_Test_data_label->hide();
    ui->R_Test_data_label->hide();
#endif

}

void MainWindow::UI_Show()
{
    ui->image_screen->show();
    ui->up_button_label->show();
    ui->down_button_label->show();
    ui->mode_tag_label->show();
    ui->mode_title_label->show();
    ui->mode_symbol_label->show();
    ui->mode_unit0_label->show();
    ui->mode_unit1_label->show();
    ui->needle_button_label->show();
    ui->pad_button_label->show();
    ui->setup_label->show();
    ui->state_title_label->show();
    ui->time_data_label->show();
    ui->time_tag_label->show();
    ui->time_title_label->show();
    ui->time_unit_label->show();
    ui->alarm_label->show();
    ui->error_label->show();
#ifdef __TSRF_TEST__
    ui->V_Test_title_label->show();
    ui->I_Test_title_label->show();
    ui->DAC_Test_title_label->show();
    ui->R_Test_title_label->show();
    ui->V_Test_data_label->show();
    ui->I_Test_data_label->show();
    ui->DAC_Test_data_label->show();
    ui->R_Test_data_label->show();
#endif
}

void MainWindow::slot_btnUDPressed(BTNS btn)
{
    Click_State click_state = sysParm->mRfDeviceFactors->click_state;
    QString String_Convert;
    bool ok = true;
    if(btn == BTN_UP)
    {
        switch(click_state)
        {
            case CLICK_TIMER_DATA:
                if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_PULSE)
                {
                    if(sysParm->mRfDeviceFactors->Timer_countlevel < 10)
                        sysParm->mRfDeviceFactors->Timer_countlevel +=1;
                }
                else if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_CONSTANT)
                {
                    if(sysParm->mRfDeviceFactors->Timer_countlevel < 3)
                        sysParm->mRfDeviceFactors->Timer_countlevel +=1;
                }
                break;
            case CLICK_VOLUME:
                if(sysParm->mRfDeviceFactors->Volume <10)
                {
                    sysParm->mRfDeviceFactors->Volume +=1;
#ifndef _ONPC_
                    gpioTools::MAX9768_Write(gpioTools::OM_FLITERLESS, sysParm->mRfDeviceFactors->Volume*7); //0~63
#endif
                }
                break;
            case CLICK_DATE_DAY:
                if(sysParm->mRfDeviceFactors->Date_day < 32)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_day);
                    sysParm->mRfDeviceFactors->Date_day = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Date_day +=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_day);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_DATES, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_DATE_MONTH:
                if(sysParm->mRfDeviceFactors->Date_month < 12)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_month);
                    sysParm->mRfDeviceFactors->Date_month = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Date_month +=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_month);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_MONTHS, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_DATE_YEAR:
                if(sysParm->mRfDeviceFactors->Date_year < 99)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_year);
                    sysParm->mRfDeviceFactors->Date_year = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Date_year +=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_year);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_YEARS, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_TIME_HOUR:
                if(sysParm->mRfDeviceFactors->Time_hour < 23)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_hour);
                    sysParm->mRfDeviceFactors->Time_hour = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Time_hour +=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_hour);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_HOURS, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_TIME_MINUTE:
                if(sysParm->mRfDeviceFactors->Time_min < 59)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_min);
                    sysParm->mRfDeviceFactors->Time_min = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Time_min +=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_min);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_MINUTES, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_TIME_AMPM:

                break;
            case CLICK_BRIGHTNESS:

                break;
            default:
                break;

        }
        Audio_Output->playSound(AudioOutput::SND_UP_BUTTON);
    }
    else if (btn == BTN_DOWN)
    {
        switch(click_state)
        {
            case CLICK_TIMER_DATA:
                if(sysParm->mRfDeviceFactors->Timer_countlevel > 1)
                    sysParm->mRfDeviceFactors->Timer_countlevel -=1;
                break;
            case CLICK_VOLUME:
                if(sysParm->mRfDeviceFactors->Volume > 0)
                {
                    sysParm->mRfDeviceFactors->Volume -=1;
#ifndef _ONPC_
                    gpioTools::MAX9768_Write(gpioTools::OM_FLITERLESS, sysParm->mRfDeviceFactors->Volume*7); //0~63
#endif
                }
                break;

            case CLICK_DATE_DAY:
                if(sysParm->mRfDeviceFactors->Date_day >0)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_day);
                    sysParm->mRfDeviceFactors->Date_day = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Date_day -=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_day);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_DAY, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_DATE_MONTH:
                if(sysParm->mRfDeviceFactors->Date_month >0)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_month);
                    sysParm->mRfDeviceFactors->Date_month = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Date_month -=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_month);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_MONTHS, String_Convert.toUInt(&ok, 16));
#endif
                }

                break;
            case CLICK_DATE_YEAR:
                if(sysParm->mRfDeviceFactors->Date_year >0)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_year);
                    sysParm->mRfDeviceFactors->Date_year = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Date_year -=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Date_year);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_YEARS, String_Convert.toUInt(&ok, 16));
#endif
                }


                break;
            case CLICK_TIME_HOUR:
                if(sysParm->mRfDeviceFactors->Time_hour >0)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_hour);
                    sysParm->mRfDeviceFactors->Time_hour = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Time_hour -=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_hour);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_HOURS, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_TIME_MINUTE:
                if(sysParm->mRfDeviceFactors->Time_min >0)
                {
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_min);
                    sysParm->mRfDeviceFactors->Time_min = String_Convert.toUInt();
                    sysParm->mRfDeviceFactors->Time_min -=1;
                    String_Convert.sprintf("%d", sysParm->mRfDeviceFactors->Time_min);
                    qDebug("String_Convert = %x", String_Convert.toInt(&ok,16));
#ifndef _ONPC_
                    gpioTools::MAX31343_Write(gpioTools::RTC_REGISTERS_MINUTES, String_Convert.toUInt(&ok, 16));
#endif
                }
                break;
            case CLICK_TIME_AMPM:
                break;
            case CLICK_BRIGHTNESS:
                break;
            default:
                break;
        }
        Audio_Output->playSound(AudioOutput::SND_DOWN_BUTTON);
    }
}

void MainWindow::slot_Main_ScreenIN()
{
    qDebug("Main_Screen In");
    UI_Show();
#ifndef _ONPC_
        gpioTools::RFSwitchLED_OFF();
#endif
    Display_Timer->start(50); //100
}

void MainWindow::slot_Main_screenIn()
{
    IsPressed = false;
    Set_Pad(sysParm->mRfDeviceFactors->state_pad);
    if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ALARM ||
       sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ERROR)
    {
        Set_State(sysParm->mRfDeviceFactors->current_state_system);
    }
    UI_Enable();
    UI_Show();
#ifndef _ONPC_
        gpioTools::RFSwitchLED_OFF();
#endif
    Display_Timer->start(50); //100

}

void MainWindow::LCD_Display()
{
    QString strTimer;
    if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ON)
    {
        strTimer.sprintf("%d", sysParm->tmElapsedTime);
    }
    else if( sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY
             || sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY )
    {
        strTimer.sprintf("%d", sysParm->mRfDeviceFactors->Timer_countlevel);
    }
    ui->time_data_label->setText(strTimer);
#ifdef __TSRF_TEST__
    Test_Display();
#endif

}
#ifdef __TSRF_TEST__
void MainWindow::Test_Display()
{
    QString str_display,Division_value,CQM_value;
    double cqm_calculate;
    Division_value.sprintf("%d", sysParm->mRfDeviceFactors->rfVoltageMeasured);
    ui->V_Test_data_label->setText(Division_value);
    Division_value.sprintf("%d", sysParm->mRfDeviceFactors->rfCurrentMeasured);
    ui->I_Test_data_label->setText(Division_value);
    str_display.sprintf("%d", sysParm->mRfDeviceFactors->rfImpedanceMeasured);
    ui->R_Test_data_label->setText(str_display);
    cqm_calculate = sysParm->CQM_Calculate;
    cqm_calculate = cqm_calculate + 0.5;
    //qDebug("cqm_calculate = %lf",cqm_calculate);
    CQM_value.sprintf("%d", (uint32)cqm_calculate);
    ui->CQM_Cal_Test_data_label->setText(CQM_value);
    CQM_value.sprintf("%d", sysParm->CQM_Data_Average);
    ui->CQM_Aver_Test_data_label->setText(CQM_value);
    if(sysParm->mRfDeviceFactors->watt_select == WATT_7)
    {
        ui->DAC_Test_title_label->setText("7W DAC");
        str_display.sprintf("%d", sysParm->mRfDeviceFactors->rfDACTableMeasured7);
    }
    else if(sysParm->mRfDeviceFactors->watt_select == WATT_15)
    {
        ui->DAC_Test_title_label->setText("15W DAC");
        str_display.sprintf("%d", sysParm->mRfDeviceFactors->rfDACTableMeasured15);
    }
    else if(sysParm->mRfDeviceFactors->watt_select == WATT_25)
    {
        ui->DAC_Test_title_label->setText("25W DAC");
        str_display.sprintf("%d", sysParm->mRfDeviceFactors->rfDACTableMeasured25);
    }
    else if(sysParm->mRfDeviceFactors->watt_select == WATT_50)
    {
        ui->DAC_Test_title_label->setText("50W DAC");
        str_display.sprintf("%d", sysParm->mRfDeviceFactors->rfDACTableMeasured50);
    }
     ui->DAC_Test_data_label->setText(str_display);
}
#endif
void MainWindow::UI_Enable()
{
    ui->setup_label->setPixmap(QPixmap(":/Images/button_setup_enable.png"));
    ui->time_tag_label->setPixmap(QPixmap(":/Images/time_tag_enable.png"));
    ui->up_button_label->setPixmap(QPixmap(":/Images/button_up_enable.png"));
    ui->down_button_label->setPixmap(QPixmap(":/Images/button_down_enable.png"));

    if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_PULSE)
    {
        ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_tag_enable_pulse.png"));
        ui->mode_symbol_label->setPixmap(QPixmap(":/Images/mode_tag_pulse.png"));
        ui->mode_title_label->setText("Pulse");
    }
    else if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_CONSTANT)
    {
        ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_tag_enable_constant.png"));
        ui->mode_symbol_label->setPixmap(QPixmap(":/Images/mode_tag_constant.png"));
        ui->mode_title_label->setText("Constant");
    }
}

void MainWindow::UI_Disable()
{
    ui->setup_label->setPixmap(QPixmap(":/Images/button_setup_disable.png"));
    ui->time_tag_label->setPixmap(QPixmap(":/Images/time_tag_disable.png"));
    //ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_disable.png"));
    ui->up_button_label->setPixmap(QPixmap(":/Images/button_up_disable.png"));
    ui->down_button_label->setPixmap(QPixmap(":/Images/button_down_disable.png"));

    if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_PULSE)
    {
        ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_tag_disable.png"));
        ui->mode_symbol_label->setPixmap(QPixmap(":/Images/mode_tag_pulse.png"));
        ui->mode_title_label->setText("Pulse");
    }
    else if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_CONSTANT)
    {
        ui->mode_tag_label->setPixmap(QPixmap(":/Images/mode_tag_disable.png"));
        ui->mode_symbol_label->setPixmap(QPixmap(":/Images/mode_tag_constant.png"));
        ui->mode_title_label->setText("Constant");
    }
}

void MainWindow::slot_btnShortPressed(BTNS state)
{
    //outputThread->RFStop();
    outputThread->RFStop_State = true;
}

void MainWindow::slot_btnLongPressed(BTNS state)
{
    if(sysParm->mRfDeviceFactors->Timer_countlevel !=0 &&
       sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY)
    {
        qDebug("Main rf_start");
        outputThread->RF_Relay_ON();
        //outputThread->Impedance_Check_Start();
        //outputThread->RFStart();
    }

}

void MainWindow::slot_rfStarted()
{

#ifndef _ONPC_
    gpioTools::RFSwitchLED_ON();
    Audio_Output->playSound(AudioOutput::SND_RFSTART);
#endif

    UI_Disable();
    Set_State(STATE_SYSTEM_ON);

}
void MainWindow::slot_rfStopped()
{
    if(sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_ON)
    {
        qDebug("rf_stop");
#ifndef _ONPC_
        gpioTools::RFSwitchLED_OFF();
        Audio_Output->playSound(AudioOutput::SND_RFSTOP);
#endif
        UI_Enable();
        Set_State(STATE_SYSTEM_READY);
    }
}

void MainWindow::slot_State_check()
{
    qDebug("slot State Check");
    if(sysParm->mRfDeviceFactors->state_system != STATE_SYSTEM_ON &&
       sysParm->mRfDeviceFactors->state_needle == STATE_NEEDLE_OFF ||
       sysParm->mRfDeviceFactors->state_pad == STATE_PAD_OFF )
    {
        Set_State(STATE_SYSTEM_STANDBY);
    }
    else if(sysParm->mRfDeviceFactors->state_system != STATE_SYSTEM_ON &&
            sysParm->mRfDeviceFactors->state_needle == STATE_NEEDLE_ON &&
            sysParm->mRfDeviceFactors->state_pad == STATE_PAD_ON)
    {
        Set_State(STATE_SYSTEM_READY);
    }
}

void MainWindow::slot_Display()
{
    LCD_Display();
}

void MainWindow::slot_Button_Loop()
{
        if(Label == ui->time_data_label)
        {
            if(Button_Counting > 1) //50ms
            {
                slot_btnLongPressed(BTN_RFSTARTSTOP);
                Button_Counting = 0;
            }
        }
        else if((Label == ui->up_button_label)&&(ui->up_button_label->isEnabled()) &&
                (sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY ||
                 sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY))
        {
            if(Button_Counting > 3) //50ms
            {

                sysParm->mRfDeviceFactors->click_state = CLICK_TIMER_DATA;
                slot_btnUDPressed(BTN_UP);
                Button_Counting = 0;
            }
        }
        else if((Label == ui->down_button_label)&&(ui->down_button_label->isEnabled()) &&
                (sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY ||
                 sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY))
        {
            if(Button_Counting > 3) //50ms
            {
                if(sysParm->mRfDeviceFactors->state_system != STATE_SYSTEM_ON)
                {
                    sysParm->mRfDeviceFactors->click_state = CLICK_TIMER_DATA;
                    slot_btnUDPressed(BTN_DOWN);
                }
                Button_Counting = 0;
            }
        }
        else if(Label == ui->pad_button_label &&
               (sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY ||
                sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY))
        {

            if(Button_Counting > 1) //10ms
            {
                qDebug("pad_button_click");
#ifndef _ONPC_
                Audio_Output->playSound(AudioOutput::SND_CLICK_BUTTON);
#endif
                if(sysParm->mRfDeviceFactors->state_pad == STATE_PAD_OFF)
                {
                   Set_Pad(STATE_PAD_ON);
                }
                else
                {
                   Set_Pad(STATE_PAD_OFF);
                }
                //emit slot_State_check();
                slot_State_check();
                Button_Timer->stop();
            }

        }
#if 0
        else if(Label == ui->needle_button_label &&
               (state_system == STATE_SYSTEM_STANDBY || state_system == STATE_SYSTEM_READY))
        {

            if(Button_Counting > 1) //10ms
            {
                qDebug("needle_button_click");
#ifndef _ONPC_
                Audio_Output->playSound(AudioOutput::SND_CLICK_BUTTON);
#endif
                if(state_needle == STATE_NEEDLE_OFF)
                {
                   Set_Needle(STATE_NEEDLE_ON, LENGTH_NEEDLE_98, ANGLE_NEEDLE_50 );
                }
                else
                {
                   Set_Needle(STATE_NEEDLE_OFF, LENGTH_DEFAULT, ANGLE_DEFAULT);
                }
                State_check();
                Button_Timer->stop();
            }

        }
#endif
        else if((Label == ui->mode_tag_label || Label == ui->mode_symbol_label) &&
                (sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY ||
                 sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY))
        {

            if(Button_Counting > 1) //10ms
            {
                qDebug("mode_button_click");
#ifndef _ONPC_
                Audio_Output->playSound(AudioOutput::SND_CLICK_BUTTON);
#endif
                if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_CONSTANT)
                {
                   //emit sig_Set_Mode(OutputMode::RF_DEVICE_OP_MODE_PULSE);
                    emit sig_Set_Mode(Mode_RF::MODE_RF_PULSE);
                }
                else if(outputThread->GetOutputMode() == OutputMode::RF_DEVICE_OP_MODE_PULSE)
                {
                   emit sig_Set_Mode(Mode_RF::MODE_RF_CONSTANT);
                }

                Button_Timer->stop();
            }

        }
        else if(Label == ui->setup_label &&
               (sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_STANDBY ||
                sysParm->mRfDeviceFactors->state_system == STATE_SYSTEM_READY))
        {
            if(Button_Counting > 1) //10ms
            {
                qDebug("setup_button_label_click");
#ifndef _ONPC_
                Audio_Output->playSound(AudioOutput::SND_CLICK_BUTTON);
#endif
                UI_Hide();
                Display_Timer->stop();
                Button_Timer->stop();
                emit sig_Setup_Screen();
            }
        }

        else if(Label == ui->alarm_label)
        {
            if(Button_Counting > 1) //10ms
            {
                qDebug("alarm_event_label_click");
#ifndef _ONPC_
                Audio_Output->playSound(AudioOutput::SND_ALARM);
#endif
                UI_Disable();
                sysParm->mRfDeviceFactors->current_state_system = sysParm->mRfDeviceFactors->state_system;
                sysParm->mRfDeviceFactors->state_system = STATE_SYSTEM_ALARM;
                emit sig_alarm_code(ALARM_CODE_000);
#ifndef _ONPC_
#if 0
                Control_SIC();
#endif
#endif
#ifndef __TSRF_TEST__
                qDebug("Watt = %d", sysParm->mRfDeviceFactors->rfPowerMeasured);
                qDebug("Current = %d", sysParm->mRfDeviceFactors->rfCurrentMeasured);
                qDebug("Volt = %d", sysParm->mRfDeviceFactors->rfVoltageMeasured);
                qDebug("Impedance = %d", sysParm->mRfDeviceFactors->rfImpedanceMeasured);
#endif
                //Display_Timer->stop();
                //Hide();
                Button_Timer->stop();
            }
        }
        else if(Label == ui->error_label)
        {
            if(Button_Counting > 1) //10ms
            {
                qDebug("error_event_label_click");
#ifndef _ONPC_
                Audio_Output->playSound(AudioOutput::SND_ERROR);
#endif
                UI_Disable();
                sysParm->mRfDeviceFactors->current_state_system = sysParm->mRfDeviceFactors->state_system;
                sysParm->mRfDeviceFactors->state_system = STATE_SYSTEM_ERROR;

                emit sig_error_code(ERROR_CODE_000);
                //Display_Timer->stop();
                //Hide();
                Button_Timer->stop();
            }
        }


        Button_Counting++; //10ms
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(IsPressed == false)
        {
            PressedButton = (QPushButton *)target;
            Label = (QLabel *)target;
            IsPressed = true;
            Button_Counting = 0;
            Button_Timer->setInterval(10);
            Button_Timer->start();
        }
        if(Button_Counting > 5)
        {
            PressedButton = nullptr;
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        if(IsPressed == true)
        {
            PressedButton = nullptr;
            Label = nullptr;
            IsPressed = false;
            Button_Timer->stop();
        }
    }

    return QWidget::eventFilter(target, event);
}
