#include "setup.h"
#include "ui_setup.h"

setup::setup(QWidget *parent, SysParm *sysParm) :
    QWidget(parent),
    ui(new Ui::setup)
{
    ui->setupUi(this);
    this->sysParm = sysParm;
    this->setIDstarmed(QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf"));
    ButtonTimer = new QTimer(this);
    DisplayTimer = new QTimer(this);
    Calibration_Screen = new calibration(this, sysParm);
    Calibration_Screen->close();

    SetDefault_value();
    SetDefault_function();

}

setup::~setup()
{
    delete ui;
}
void setup::SetConnect()
{
    connect(ButtonTimer, SIGNAL(timeout()), this, SLOT(slot_Button_Loop()));
    connect(DisplayTimer, SIGNAL(timeout()), this, SLOT(slot_Display()));
    connect(this, SIGNAL(sig_ScreenIn_Calibration(Call_State)), Calibration_Screen, SLOT(slot_ScreenIn_Calibration(Call_State)));
    connect(Calibration_Screen, SIGNAL(sig_ScreenIn_Setup()), this, SLOT(slot_Setup_Screenin()));

}
void setup::slot_Setup_ScreenIN()
{
    qDebug("Setup Screen IN");
#ifndef _ONPC_
        gpioTools::RFSwitchLED_OFF();
#endif

    Choose_Click_state(CLICK_NONE);
    Set_Mode(sysParm->mRfDeviceFactors->mode_rf);
    DisplayTimer->start(50);
    IsPressed = false;
    LED_State = true;
    LED_Count = 0;
    this->show();
}
void setup::slot_Setup_Screenin()
{
    qDebug("Setup Screen in");
    SetDefault_value();
    Choose_Click_state(CLICK_NONE);
    DisplayTimer->start(50);
    IsPressed = false;
    LED_State = true;
    LED_Count = 0;
    UI_Show();
}

void setup::SetDefault_value()
{
    IsPressed = false;
    State_Mode = false;
    State_Language = false;

    /*
    LED_State = true;
    LED_Count = 0;
    LED_Brightness_State = true;
    LED_Brightness_Add = 0;
    LED_Brightness_Sub = 250;
    LED_Brightness_Count = 0;

    color_state = LED_COLOR_WHITE;
    point_state = LED_POINT_MIDLE;

    state_needle = STATE_NEEDLE_OFF;
    state_pad = STATE_PAD_OFF;
    state_system = STATE_SYSTEM_STANDBY;
    mode_rf = MODE_RF_CONSTANT;
    */

}



void setup::SetDefault_function()
{
    SetImage();
    SetUI_Color();
    SetFont();
    SetConnect();
    SetEventFilter();

}


void setup::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont title(family,30, QFont::Bold);
    QFont big_title(family,50, QFont::Bold);

    QFont button(family,20, QFont::Bold);
    QFont uinit(family,50, QFont::Bold);
    QFont data(family,35, QFont::Bold);
    QFont sw_data(family,30, QFont::Bold);

    QFont smalldata(family,15, QFont::Bold);

    QFont symbol(family,15, QFont::Bold);
    QFont mode(family,20, QFont::Bold);
    QFont language(family,20, QFont::Bold);

    //label size
    ui->volume_title_label->setFont(title);
    ui->date_title_label->setFont(title);
    ui->startup_title_label->setFont(big_title);
    ui->timer_title_label->setFont(title);
    ui->mode_title_label->setFont(title);
    ui->language_title_label->setFont(title);
    ui->sw_title_label->setFont(title);
    ui->brightness_title_label->setFont(title);


    //data label size
    ui->volume_data_label->setFont(data);
    ui->date_day_label->setFont(smalldata);
    ui->date_month_label->setFont(smalldata);
    ui->date_year_label->setFont(smalldata);
    ui->time_hour_label->setFont(smalldata);
    ui->time_minute_label->setFont(smalldata);
    ui->time_ampm_label->setFont(smalldata);

    ui->brightness_data_label->setFont(data);


    ui->timer_data_label->setFont(data);

    ui->sw_data_label->setFont(sw_data);

    ui->timer_symbol_label->setFont(data);
    ui->date_symbol1_label->setFont(symbol);
    ui->date_symbol2_label->setFont(symbol);
    ui->time_symbol1_label->setFont(symbol);
    ui->time_symbol2_label->setFont(symbol);

    ui->mode_data_label->setFont(mode);
    ui->language_data_label->setFont(language);


    //title color
    ui->volume_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->date_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->startup_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->timer_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->mode_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->language_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->sw_title_label->setStyleSheet("QLabel { color : black; }"); //text color
    ui->brightness_title_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->date_symbol1_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->date_symbol2_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->time_symbol1_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->time_symbol2_label->setStyleSheet("QLabel { color : white; }"); //text color
    ui->timer_symbol_label->setStyleSheet("QLabel { color : white; }"); //text color

    //data color
    ui->sw_data_label->setStyleSheet("QLabel { color : blue; }"); //text color
    ui->timer_data_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->volume_data_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->date_year_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->date_month_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->date_day_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->time_hour_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->time_minute_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->time_ampm_label->setStyleSheet("QLabel { color : cyan; }"); //text color
    ui->brightness_data_label->setStyleSheet("QLabel { color : cyan; }"); //text color

    //title text
    ui->volume_title_label->setText("Volume");
    ui->startup_title_label->setText("Setup");
    ui->timer_title_label->setText("Timer");
    ui->brightness_title_label->setText("Brightness");

    ui->volume_title_label->setAlignment(Qt::AlignCenter);
    ui->timer_title_label->setAlignment(Qt::AlignCenter);
    ui->mode_title_label->setAlignment(Qt::AlignCenter);
    ui->brightness_title_label->setAlignment(Qt::AlignCenter);
    ui->date_title_label->setAlignment(Qt::AlignCenter);
    ui->language_title_label->setAlignment(Qt::AlignCenter);


    ui->mode_data_label->setAlignment(Qt::AlignCenter);
    ui->language_data_label->setAlignment(Qt::AlignCenter);
    ui->timer_data_label->setAlignment(Qt::AlignCenter);
    ui->volume_data_label->setAlignment(Qt::AlignCenter);
    ui->date_day_label->setAlignment(Qt::AlignCenter);
    ui->date_month_label->setAlignment(Qt::AlignCenter);
    ui->date_year_label->setAlignment(Qt::AlignCenter);
    ui->date_symbol1_label->setAlignment(Qt::AlignCenter);
    ui->date_symbol2_label->setAlignment(Qt::AlignCenter);
    ui->time_hour_label->setAlignment(Qt::AlignCenter);
    ui->time_minute_label->setAlignment(Qt::AlignCenter);
    ui->time_ampm_label->setAlignment(Qt::AlignCenter);
    ui->time_symbol1_label->setAlignment(Qt::AlignCenter);
    ui->time_symbol2_label->setAlignment(Qt::AlignCenter);

}
void setup::SetUI_Color()
{

    //ui->mode_button->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,   stop:0 rgba(50, 50, 255, 100), stop:1 rgba(20, 50, 150, 150))");
    //ui->language_button->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,   stop:0 rgba(200, 200, 50, 100), stop:1 rgba(150, 150, 50, 150))");

    //ui->setup_button->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,   stop:0 rgba(20, 20, 20, 100), stop:1 rgba(20, 20, 20, 150))");
    //ui->up_button->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,   stop:0 rgba(60, 255, 50, 100), stop:1 rgba(98, 255, 150, 150))");
    //ui->down_button->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,   stop:0 rgba(60, 255, 100, 100), stop:1 rgba(98, 255, 150, 150))");

    //ui->Result_frame->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,   stop:0 rgba(60, 255, 50, 50), stop:1 rgba(98, 255, 150, 30))");
}

void setup::SetImage()
{
    ui->image_screen->setPixmap(QPixmap(":/Images/state_setup.jpg"));
    ui->back_label->setPixmap(QPixmap(":/Images/button_back_enable.png"));
    ui->up_button_label->setPixmap(QPixmap(":/Images/button_up_enable.png"));
    ui->down_button_label->setPixmap(QPixmap(":/Images/button_down_enable.png"));
    ui->mode_button_label->setPixmap(QPixmap(":/Images/button_setup_pulse.png"));
    ui->language_button_label->setPixmap(QPixmap(":/Images/button_setup_language.png"));
    ui->timer_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->volume_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->brightness_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->date_day_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->date_month_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->date_year_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->time_hour_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->time_miute_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));
    ui->time_ampm_highlight_label->setPixmap(QPixmap(":/Images/highlight.png"));

    ui->image_screen->setScaledContents(true);
    ui->back_label->setScaledContents(true);
    ui->up_button_label->setScaledContents(true);
    ui->down_button_label->setScaledContents(true);
    ui->mode_button_label->setScaledContents(true);
    ui->language_button_label->setScaledContents(true);

    ui->timer_highlight_label->setScaledContents(true);
    ui->volume_highlight_label->setScaledContents(true);
    ui->brightness_highlight_label->setScaledContents(true);
    ui->date_day_highlight_label->setScaledContents(true);
    ui->date_month_highlight_label->setScaledContents(true);
    ui->date_year_highlight_label->setScaledContents(true);
    ui->time_hour_highlight_label->setScaledContents(true);
    ui->time_miute_highlight_label->setScaledContents(true);
    ui->time_ampm_highlight_label->setScaledContents(true);


}

void setup::SetEventFilter()
{

    ui->back_label->installEventFilter(this);

    ui->date_day_label->installEventFilter(this);
    ui->date_month_label->installEventFilter(this);
    ui->date_year_label->installEventFilter(this);
    ui->time_hour_label->installEventFilter(this);
    ui->time_ampm_label->installEventFilter(this);
    ui->time_minute_label->installEventFilter(this);
    ui->volume_data_label->installEventFilter(this);
    ui->timer_data_label->installEventFilter(this);
    ui->brightness_data_label->installEventFilter(this);

    ui->up_button_label->installEventFilter(this);
    ui->down_button_label->installEventFilter(this);

    ui->mode_button_label->installEventFilter(this);
    ui->language_button_label->installEventFilter(this);
    ui->mode_data_label->installEventFilter(this);
    ui->language_data_label->installEventFilter(this);

    ui->startup_title_label->installEventFilter(this);

}
void setup::Set_Mode(Mode_RF mode)
{
    sysParm->mRfDeviceFactors->mode_rf = mode;
    switch(mode)
    {
        case Mode_RF::MODE_RF_PULSE:
            emit sig_Set_Mode(Mode_RF::MODE_RF_PULSE);
            ui->mode_data_label->setText("Pulse");
            ui->mode_button_label->setPixmap(QPixmap(":/Images/button_setup_pulse.png"));
            break;
        case Mode_RF::MODE_RF_CONSTANT:
            emit sig_Set_Mode(Mode_RF::MODE_RF_CONSTANT);
            ui->mode_data_label->setText("Constant");
            ui->mode_button_label->setPixmap(QPixmap(":/Images/button_setup_constant.png"));
            break;

        default :
            break;
    }
}
void setup::Set_Language()
{
    if(State_Language == true)
    {
        ui->language_data_label->setText("English");
        ui->language_button_label->setPixmap(QPixmap(":/Images/button_setup_language.png"));
    }
    else
    {
        ui->language_data_label->setText("Japanese");
        ui->language_button_label->setPixmap(QPixmap(":/Images/button_setup_language1.png"));
    }
    State_Language ^= true;

}

void setup::Choose_Click_state(Click_State state)
{
    sysParm->mRfDeviceFactors->click_state = state;
    switch(state)
    {
        case CLICK_NONE:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();
            break;

        case CLICK_TIMER_DATA:
            ui->timer_highlight_label->show();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();
            break;

        case CLICK_VOLUME:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->show();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();
            break;
        case CLICK_DATE_DAY:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->show();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();
            break;
        case CLICK_DATE_MONTH:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->show();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();

            break;
        case CLICK_DATE_YEAR:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->show();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();

            break;
        case CLICK_TIME_HOUR:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->show();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();
            break;
        case CLICK_TIME_MINUTE:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->show();
            ui->time_ampm_highlight_label->hide();
            break;
        case CLICK_TIME_AMPM:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->hide();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->show();

            break;
        case CLICK_BRIGHTNESS:
            ui->timer_highlight_label->hide();
            ui->volume_highlight_label->hide();
            ui->brightness_highlight_label->show();
            ui->date_day_highlight_label->hide();
            ui->date_month_highlight_label->hide();
            ui->date_year_highlight_label->hide();
            ui->time_hour_highlight_label->hide();
            ui->time_miute_highlight_label->hide();
            ui->time_ampm_highlight_label->hide();
            break;

        default:
            break;
   }
}



void setup::Display_Front_RGBLED(LED_Color color, LED_Point point, int32 brightness)
{
    //qDebug("slot_LED_Display Test");
    uint32 point_arr[3];
#ifndef _ONPC_
#if 0
    gpioTools::LED_I2C_Open();
    gpioTools::LP5009_Write(gpioTools::LED_COMMANDS_OUT0_COLOR, 0xFF);
    gpioTools::LP5009_Write(gpioTools::LED_COMMANDS_OUT3_COLOR, 0xFF);
    gpioTools::LP5009_Write(gpioTools::LED_COMMANDS_OUT6_COLOR, 0xFF);
    gpioTools::LED_I2C_Close();
#endif
    gpioTools::LED_I2C_Open();
    //qDebug("LED Color = %d, LED Point = %d", color ,point );
    switch(point)
    {
        case LED_POINT_LEFT:
                point_arr[0] = gpioTools::LED_COMMANDS_OUT0_COLOR;
                point_arr[1] = gpioTools::LED_COMMANDS_OUT1_COLOR;
                point_arr[2] = gpioTools::LED_COMMANDS_OUT2_COLOR;

            break;
        case LED_POINT_MIDLE:
                point_arr[0] = gpioTools::LED_COMMANDS_OUT3_COLOR;
                point_arr[1] = gpioTools::LED_COMMANDS_OUT4_COLOR;
                point_arr[2] = gpioTools::LED_COMMANDS_OUT5_COLOR;
            break;
        case LED_POINT_RIGHT:
                point_arr[0] = gpioTools::LED_COMMANDS_OUT6_COLOR;
                point_arr[1] = gpioTools::LED_COMMANDS_OUT7_COLOR;
                point_arr[2] = gpioTools::LED_COMMANDS_OUT8_COLOR;
            break;
        default :
            point_arr[0] = 0xff;
            point_arr[1] = 0xff;
            point_arr[2] = 0xff;
            break;
    }


    switch(color)
    {
        case LED_COLOR_WHITE:
                gpioTools::LP5009_Write(point_arr[0], brightness); //0xFF
                gpioTools::LP5009_Write(point_arr[1], brightness); //0xFF
                gpioTools::LP5009_Write(point_arr[2], brightness); //0xFF
            break;
        case LED_COLOR_GEERN:
                gpioTools::LP5009_Write(point_arr[0], 0x00);
                gpioTools::LP5009_Write(point_arr[1], 0xFF);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;
        case LED_COLOR_BLUE:
                gpioTools::LP5009_Write(point_arr[0], 0x00);
                gpioTools::LP5009_Write(point_arr[1], 0x00);
                gpioTools::LP5009_Write(point_arr[2], 0xFF);
            break;

        case LED_COLOR_ORANGE:
                gpioTools::LP5009_Write(point_arr[0], 0xFF);
                gpioTools::LP5009_Write(point_arr[1], 0x5E);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;

        case LED_COLOR_RED:
                gpioTools::LP5009_Write(point_arr[0], 0xFF);
                gpioTools::LP5009_Write(point_arr[1], 0x00);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;
        case LED_COLOR_GRAY:
                gpioTools::LP5009_Write(point_arr[0], 0x32);
                gpioTools::LP5009_Write(point_arr[1], 0x32);
                gpioTools::LP5009_Write(point_arr[2], 0x32);
            break;


        case LED_COLOR_DEFAULT: //LED Off
                gpioTools::LP5009_Write(point_arr[0], 0x00);
                gpioTools::LP5009_Write(point_arr[1], 0x00);
                gpioTools::LP5009_Write(point_arr[2], 0x00);
            break;

        default :
            break;
    }


    gpioTools::LED_I2C_Close();
#endif

}
void setup::LCD_Display()
{
    QString strText;

    strText.sprintf("%d", sysParm->mRfDeviceFactors->Timer_countlevel);
    ui->timer_data_label->setText(strText);
    strText.sprintf("%d", sysParm->mRfDeviceFactors->Volume);
    ui->volume_data_label->setText(strText);
    strText.sprintf("%d", sysParm->mRfDeviceFactors->Date_year);
    ui->date_year_label->setText(strText);
    strText.sprintf("%d", sysParm->mRfDeviceFactors->Date_month);
    ui->date_month_label->setText(strText);
    strText.sprintf("%d", sysParm->mRfDeviceFactors->Date_day);
    ui->date_day_label->setText(strText);
    strText.sprintf("%d", sysParm->mRfDeviceFactors->Time_hour);
    ui->time_hour_label->setText(strText);
    strText.sprintf("%d", sysParm->mRfDeviceFactors->Time_min);
    ui->time_minute_label->setText(strText);

}
void setup::Control_RGBLED()
{

    if(LED_State == true) //LED On
    {
        Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_LEFT,  0);
        Display_Front_RGBLED(color_state, point_state, 0);
        Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_RIGHT, 0);
    }
    else //LED Off
    {
        Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_LEFT,  0);
        Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_MIDLE, 0);
        Display_Front_RGBLED(LED_COLOR_DEFAULT, LED_POINT_RIGHT, 0);
    }
    LED_Count ++;
    if(color_state != LED_COLOR_WHITE && LED_Count >= 20)
    {
        LED_State ^= true;
        LED_Count = 0;
    }
    else if (color_state == LED_COLOR_WHITE && LED_Count >= 40)
    {
        LED_State = false;
        LED_Count = 0;
    }

}

void setup::UI_Show()
{
    ui->back_label->show();
    ui->brightness_data_label->show();

    ui->brightness_title_label->show();
    ui->date_day_label->show();

    ui->date_month_label->show();

    ui->date_symbol1_label->show();
    ui->date_symbol2_label->show();
    ui->date_title_label->show();
    ui->date_year_label->show();

    ui->down_button_label->show();

    ui->image_screen->show();
    ui->language_button_label->show();
    ui->language_data_label->show();
    ui->language_title_label->show();

    ui->mode_button_label->show();
    ui->mode_data_label->show();
    ui->mode_title_label->show();
    ui->startup_title_label->show();
    ui->sw_data_label->show();
    ui->sw_title_label->show();
    ui->timer_data_label->show();

    ui->timer_symbol_label->show();
    ui->timer_title_label->show();
    ui->time_ampm_label->show();

    ui->time_hour_label->show();

    ui->time_minute_label->show();

    ui->time_symbol1_label->show();
    ui->time_symbol2_label->show();
    ui->up_button_label->show();
    ui->volume_data_label->show();

    ui->volume_title_label->show();

    ui->line->show();
    ui->line_2->show();
    ui->line_3->show();
    ui->line_4->show();
    ui->line_5->show();
    ui->line_6->show();
    ui->line_7->show();

}
void setup::UI_Hide()
{
    ui->back_label->hide();
    ui->brightness_data_label->hide();

    ui->brightness_title_label->hide();
    ui->date_day_label->hide();

    ui->date_month_label->hide();

    ui->date_symbol1_label->hide();
    ui->date_symbol2_label->hide();
    ui->date_title_label->hide();
    ui->date_year_label->hide();

    ui->down_button_label->hide();

    ui->image_screen->hide();
    ui->language_button_label->hide();
    ui->language_data_label->hide();
    ui->language_title_label->hide();

    ui->mode_button_label->hide();
    ui->mode_data_label->hide();
    ui->mode_title_label->hide();
    ui->startup_title_label->hide();
    ui->sw_data_label->hide();
    ui->sw_title_label->hide();
    ui->timer_data_label->hide();

    ui->timer_symbol_label->hide();
    ui->timer_title_label->hide();
    ui->time_ampm_label->hide();

    ui->time_hour_label->hide();

    ui->time_minute_label->hide();

    ui->time_symbol1_label->hide();
    ui->time_symbol2_label->hide();
    ui->up_button_label->hide();
    ui->volume_data_label->hide();

    ui->volume_title_label->hide();

    ui->line->hide();
    ui->line_2->hide();
    ui->line_3->hide();
    ui->line_4->hide();
    ui->line_5->hide();
    ui->line_6->hide();
    ui->line_7->hide();

}
void setup::slot_Display()
{
    //color_state = LED_COLOR_GRAY;
    //point_state = LED_POINT_MIDLE;

    //Control_RGBLED();
    LCD_Display();
}


void setup::slot_Button_Loop()
{
        if((Label == ui->up_button_label)&&(ui->up_button_label->isEnabled()))
        {
            if(Button_Counting > 3) //30ms
            {
                emit sig_btnUDPressed(BTN_UP);
                Button_Counting = 0;
            }
        }
        else if((Label == ui->down_button_label)&&(ui->down_button_label->isEnabled()))
        {
            if(Button_Counting > 3) //30ms
            {
                emit sig_btnUDPressed(BTN_DOWN);
                Button_Counting = 0;
            }
        }
        else if(Label == ui->back_label)
        {
            if(Button_Counting > 1) //10ms
            {
                qDebug("back_button_label_click");
                emit sig_Main_Screen();
                //display_Timer->stop();
                //Hide();
                DisplayTimer->stop();
                ButtonTimer->stop();
                this->close();
            }
        }
        else if(Label == ui->startup_title_label)
        {
            if(Button_Counting > 1) //10ms
            {
                qDebug("Calibration Click");
                UI_Hide();
                Choose_Click_state(CLICK_NONE);
                DisplayTimer->stop();
                ButtonTimer->stop();
                emit sig_ScreenIn_Calibration(CALL_FROME_SETUP);
            }
        }
        else if(Label == ui->mode_button_label || Label == ui->mode_data_label)
        {
            if(Button_Counting > 3) //10ms
            {
                qDebug("mode_button_label_click");
                //Set_Mode();
                if(sysParm->mRfDeviceFactors->mode_rf == Mode_RF::MODE_RF_CONSTANT)
                {
                    Set_Mode(Mode_RF::MODE_RF_PULSE);
                }
                else if(sysParm->mRfDeviceFactors->mode_rf == Mode_RF::MODE_RF_PULSE)
                {
                    Set_Mode(Mode_RF::MODE_RF_CONSTANT);
                }
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->language_button_label || Label == ui->language_data_label)
        {
            if(Button_Counting > 3) //10ms
            {
                qDebug("language_button_label_click");
                Set_Language();
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->timer_data_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_TIMER_DATA);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->volume_data_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_VOLUME);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->date_day_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_DATE_DAY);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->date_month_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_DATE_MONTH);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->date_year_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_DATE_YEAR);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->time_hour_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_TIME_HOUR);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->time_minute_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_TIME_MINUTE);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->time_ampm_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_TIME_AMPM);
                ButtonTimer->stop();
            }
        }
        else if(Label == ui->brightness_data_label)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_BRIGHTNESS);
                ButtonTimer->stop();
            }
        }
        Button_Counting++; //10ms
}

bool setup::eventFilter(QObject *target, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(IsPressed == false)
        {
            PressedButton = (QPushButton *)target;
            Label = (QLabel *)target;
            IsPressed = true;
            Button_Counting = 0;
            ButtonTimer->setInterval(10);
            ButtonTimer->start();
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
            ButtonTimer->stop();
        }
    }

    return QWidget::eventFilter(target, event);
}
