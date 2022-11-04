#include "calibration.h"
#include "ui_calibration.h"
#include <QFontDatabase>
#include <QDebug>
#include <QFont>

calibration::calibration(QWidget *parent, SysParm *sysParm) :
    QWidget(parent),
    ui(new Ui::calibration)
{
    ui->setupUi(this);
    this->sysParm = sysParm;

    Button_Timer = new QTimer(this);
    DisplayTimer = new QTimer(this);
    DisplayTimer->setInterval(50);
    this->setIDstarmed(QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf"));
    ui->RF_button->setPalette(Ui_SetColor(QPalette::ButtonText, Qt::white, QPalette::Button,Qt::blue));
    IsPressed = false;
    IsUSBstate = false;
    SetFont();
    Calibration_connect();
    installFilter();
    //sysParm->mRfDeviceFactors->v_division_level = 180; //210
    //sysParm->mRfDeviceFactors->i_division_level = 150; //110

}

calibration::~calibration()
{
    delete ui;
}

void calibration::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont label_font(family,25);
    QFont symbol_font(family,20);

    QFont serial_label_font(family,20);
    QFont button_font(family,30);
    QFont main_font(family,30);
    ui->Modelabel->setFont(main_font);
    ui->Back_button->setFont(button_font);
    ui->Next_button->setFont(button_font);
    ui->Up_button->setFont(button_font);
    ui->Down_button->setFont(button_font);
    ui->USB_button->setFont(button_font);
    ui->RF_button->setFont(button_font);
    ui->DAC_label->setFont(label_font);
    ui->V_label->setFont(label_font);
    ui->I_label->setFont(label_font);
    ui->W_label->setFont(label_font);
    ui->R_label->setFont(label_font);
    ui->sw_label->setFont(label_font);
    ui->SN_label->setFont(label_font);
    ui->SW_label->setFont(label_font);
    ui->serial_label->setFont(serial_label_font);



    ui->V_result_symbol_label->setFont(symbol_font);
    ui->I_result_symbol_label->setFont(symbol_font);
    ui->W_symbol_label->setFont(symbol_font);
    ui->R_symbol_label->setFont(symbol_font);
    ui->Average_label->setFont(label_font);

}

void calibration::Choose_Click_state(Click_State state)
{
    sysParm->mRfDeviceFactors->mode_click_state = state;
    switch(state)
    {
        case CLICK_NONE:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DAC_UNITS:
            ui->DAC_units_line->show();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DAC_TENS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->show();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DAC_HUNDREDS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->show();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DAC_THOUSANDS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->show();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DAC_TENTHOUSANDS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->show();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DIVISION_V_UNITS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->show();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DIVISION_V_TENS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->show();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DIVISION_V_HUNDREDS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->show();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;

        case CLICK_CALIBRATION_DIVISION_I_UNITS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->show();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DIVISION_I_TENS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->show();
            ui->I_hundreds_line->hide();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_DIVISION_I_HUNDREDS:
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->show();
            ui->Aver_line->hide();
            break;
        case CLICK_CALIBRATION_AVERAGE_COUNT:
            ui->Aver_line->show();
            ui->DAC_units_line->hide();
            ui->DAC_tens_line->hide();
            ui->DAC_hundreds_line->hide();
            ui->DAC_thousands_line->hide();
            ui->DAC_tenthousands_line->hide();
            ui->V_units_line->hide();
            ui->V_tens_line->hide();
            ui->V_hundreds_line->hide();
            ui->I_units_line->hide();
            ui->I_tens_line->hide();
            ui->I_hundreds_line->hide();

            break;

        default:
            break;
   }
}

void calibration::Calibration_connect()
{
      connect(DisplayTimer,SIGNAL(timeout()), this, SLOT(slot_display()));
      connect(Button_Timer, SIGNAL(timeout()), this, SLOT(slot_Button_Loop()));
      connect(this, SIGNAL(sig_btnReleasedCal(int)), this, SLOT(slot_btnReleasedCal(int)));
      connect(this, SIGNAL(sig_btnDirReleasedCal(int)), this, SLOT(slot_btnDirReleasedCal(int)));





}

void calibration::slot_ScreenIn_Calibration(Call_State State)
{
    switch(State)
    {
        case CALL_FROME_SETUP:
            this->show();
            break;
        case CALL_FROME_CALIBRATION:
            UI_Show();
            break;
        case CALL_FROME_ENGINEER:
            UI_Show();
            break;


        default :
            break;
    }
    IsPressed = false;
    sysParm->mRfDeviceFactors->tmDACValueToSetLast = 0;
    data_in_index = 0;
    DisplayTimer->start();
    Choose_Click_state(CLICK_NONE);

}

void calibration::slot_display()
{
    QString Serial_No_Print,temp, DAC_value, Division_level_v, Division_level_i, Division_value , Watt_value , Impedance_value;
    QString SW_Print, Average_count, Average_data;
    //sysParm->mRfDeviceFactors->rfPowerMeasured/100;
    //sysParm->mRfDeviceFactors->rfImpedanceMeasured;

    uint16 V_Real= sysParm->mRfDeviceFactors->tmReadADCVoltage;//sysParm->mRfDeviceFactors->tmReadADCVoltage;
    uint16 I_Real= sysParm->mRfDeviceFactors->tmReadADCCurrent;//sysParm->mRfDeviceFactors->tmReadADCCurrent;
    uint16 V_div_level = sysParm->mRfDeviceFactors->v_division_level;
    uint16 I_div_level = sysParm->mRfDeviceFactors->i_division_level;
    uint16 Average_V = sysParm->mRfDeviceFactors->tmAverReadADCVoltage;
    uint16 Average_I = sysParm->mRfDeviceFactors->tmAverReadADCCurrent;


    double V_div_value = (double)V_Real/V_div_level;
    double I_div_value = (double)I_Real/I_div_level;
    double Watt = V_div_value * (I_div_value/1000);
    //uint16 Watt = V_Real * (I_Real/1000);
    uint16 Impedance = V_div_value / (I_div_value/1000);
    //uint16 Impedance = V_Real / (I_Real/1000);




    //DAC.sprintf("%02d", sysParm->mRfDeviceFactors->tmDACValueToSetforCal);

    //ui->DAC_tens_LCD->display(sysParm->mRfDeviceFactors->tmDACValueToSetforCal);
    DAC_value.sprintf("%d", sysParm->mRfDeviceFactors->tmDACValueToSetforCal);
    ui->DAC_units_LCD->display(DAC_value);
    DAC_value.sprintf("%d", sysParm->mRfDeviceFactors->tmDACValueToSetforCal/10);
    ui->DAC_tens_LCD->display(DAC_value);
    DAC_value.sprintf("%d", sysParm->mRfDeviceFactors->tmDACValueToSetforCal/100);
    ui->DAC_hundreds_LCD->display(DAC_value);
    DAC_value.sprintf("%d", sysParm->mRfDeviceFactors->tmDACValueToSetforCal/1000);
    ui->DAC_thousands_LCD->display(DAC_value);
    DAC_value.sprintf("%d", sysParm->mRfDeviceFactors->tmDACValueToSetforCal/10000);
    ui->DAC_tenthousands_LCD->display(DAC_value);

    Division_level_v.sprintf("%d", V_div_level);
    ui->V_units_LCD->display(Division_level_v);
    Division_level_v.sprintf("%d", V_div_level/10);
    ui->V_tens_LCD->display(Division_level_v);
    Division_level_v.sprintf("%d", V_div_level/100);
    ui->V_hundreds_LCD->display(Division_level_v);

    Division_level_i.sprintf("%d", I_div_level);
    ui->I_units_LCD->display(Division_level_i);
    Division_level_i.sprintf("%d", I_div_level/10);
    ui->I_tens_LCD->display(Division_level_i);
    Division_level_i.sprintf("%d", I_div_level/100);
    ui->I_hundreds_LCD->display(Division_level_i);


    Division_value.sprintf("%.2lf", V_div_value);
    ui->V_result_LCD->display(Division_value);
    Division_value.sprintf("%.2lf", I_div_value);
    ui->I_result_LCD->display(Division_value);

    /*
    Division_value.sprintf("%d", V_Real);
    ui->V_result_LCD->display(Division_value);
    Division_value.sprintf("%d", I_Real);
    ui->I_result_LCD->display(Division_value);
    */

    Watt_value.sprintf("%.2lf", Watt);
    //Watt_value.sprintf("%d", Watt);
    Impedance_value.sprintf("%d", Impedance);
    ui->W_LCD->display(Watt_value);
    ui->R_LCD->display(Impedance_value);
    ui->V_LCD->display(V_Real);
    ui->I_LCD->display(I_Real);

    Average_count.sprintf("%d", sysParm->mRfDeviceFactors->average_count);
    ui->Aver_LCD->display(Average_count);

    Average_data.sprintf("%d", Average_V);
    ui->V_aver_LCD->display(Average_data);
    Average_data.sprintf("%d", Average_I);
    ui->I_aver_LCD->display(Average_data);
#ifndef _ONPC_
    if(sysParm->isDoTestforCal == true)
    {
        qDebug("Average_V = %d" , Average_V);
        qDebug("Average_I = %d" , Average_I);
    }
#endif
#if 0
    if(sysParm->mRfDeviceFactors->mode_rf == MODE_RF_CONSTANT)
    {
        ui->W7_button->hide();
        ui->W15_button->hide();
        ui->W25_button->show();
        ui->W50_button->show();

    }
    else if(sysParm->mRfDeviceFactors->mode_rf == MODE_RF_PULSE)
    {
        ui->W7_button->show();
        ui->W15_button->show();
        ui->W25_button->hide();
        ui->W50_button->hide();
    }
#endif
    /*
    Division_value.sprintf("%d", );
    ui->V_division_LCD->display(Division_level);
    */

    //ui->W_LCD->display(Power_M);
    //ui->R_LCD->display(Impedance_M);

    //temp.sprintf(" %d.%d ", sysParm->mRfDeviceFactors->rfTemperatureMeasuredA/10,sysParm->mRfDeviceFactors->rfTemperatureMeasuredA%10);
    //temp.sprintf("%d", sysParm->mRfDeviceFactors->rfTemperatureMeasuredA/10);

    //qDebug() << "sysParm->mRfDeviceFactors->tmDACValueToSetforCal = " << sysParm->mRfDeviceFactors->tmDACValueToSetforCal ;
    //qDebug() << "mRfDeviceFactors->rfTemperatureMeasuredA = " << sysParm->mRfDeviceFactors->rfTemperatureMeasuredA ;
    if(IsUSBstate == true)
    {
#ifndef _ONPC_
        gpioTools::uart_printf("%d,%d,%d,%d,%d,%d,%d\n",
                data_in_index++,
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal,
                sysParm->mRfDeviceFactors->tmReadADCVoltage,
                sysParm->mRfDeviceFactors->tmReadADCCurrent,
                sysParm-> mRfDeviceFactors->rfVoltageMeasured,
                sysParm->mRfDeviceFactors->rfCurrentMeasured,
                sysParm->mRfDeviceFactors->rfImpedanceMeasured              );
#else
        qDebug("%d,%d,%d,%d,%d,%d,%d\n",
                data_in_index++,
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal,
                sysParm->mRfDeviceFactors->tmReadADCVoltage,
                sysParm->mRfDeviceFactors->tmReadADCCurrent,
                sysParm-> mRfDeviceFactors->rfVoltageMeasured,
                sysParm->mRfDeviceFactors->rfCurrentMeasured,
                sysParm->mRfDeviceFactors->rfImpedanceMeasured              );
#endif
    }
    if(!sysParm->isDoTestforCal )
    {
        if(IsUSBstate == false)
        {
            //Limited_Button();
        }

    }
}

QPalette calibration::Ui_SetColor(QPalette::ColorRole Ui_Text, Qt::GlobalColor Text_color,
                              QPalette::ColorRole Ui, Qt::GlobalColor color)
{
    QPalette Ui_Color = palette();
    Ui_Color.setBrush(Ui_Text, Text_color);
    Ui_Color.setBrush(Ui, color);
    return Ui_Color;
}

void calibration::slot_btnReleasedCal(int keyEvent)
{
    int32 uart_fd;
    switch(keyEvent)
    {

        case BTNS::BTN_RFSTARTSTOP:
            if(!sysParm->isDoTestforCal )
            {
                qDebug("Enable Amp");
                ui->RF_button->setPalette(Ui_SetColor(QPalette::ButtonText, Qt::white, QPalette::Button,Qt::red));
                data_in_index = 0;
                sysParm-> mRfDeviceFactors->tmDACValueToSetLast = sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
                sysParm->isDoTestforCal = true;
                gpioTools::EnableAmplifierPowerOutput();
                //gpioTools::EnableIsolated_DCDC();
                //gpioTools::EnableTargetRelay();

                //gpioTools::SetAmplifierPowerOutputDirection(gpioTools::RFOUTPUT_DIR_TARGET);
                //gpioTools::SetElectrodeType(gpioTools::ELECTRODE_MONOPOLAR);
                qDebug("ReadGPIOPort = %d" ,gpioTools::ReadGPIOPort(gpioTools::GPIO_3, 31));
                sysParm->mRfDeviceFactors->counting = 0;
                //sysParm->mRfDeviceFactors->TEST_DACValueToSetLast = sysParm->mRfDeviceFactors->tmDACValueToSetLast;
            }
            else
            {
                qDebug("Disable Amp");
                ui->RF_button->setPalette(Ui_SetColor(QPalette::ButtonText, Qt::white, QPalette::Button,Qt::blue));
                sysParm->mRfDeviceFactors->tmDACValueToSetLast =  0;
                //gpioTools::DACPortOutput(sysParm->mRfDeviceFactors->tmDACValueToSetLast);

                //gpioTools::SetElectrodeType(gpioTools::ELECTRODE_NONE);
                //gpioTools::SetAmplifierPowerOutputDirection(gpioTools::RFOUTPUT_DIR_TARGET);
                gpioTools::DisableAmplifierPowerOutput();
                //gpioTools::DisableIsolated_DCDC();
                //gpioTools::DisableTargetRelay();

                qDebug("ReadGPIOPort = %d" ,gpioTools::ReadGPIOPort(gpioTools::GPIO_3, 31));
                sysParm->isDoTestforCal = false;
            }
            break;

        case BTNS::BTN_MODE:
            //qDebug("BTN_USB");
            if(IsUSBstate == true)
            {
                data_in_index = 0;
                ui->RF_button->setDisabled(true);
                ui->Up_button->setDisabled(true);
                ui->Down_button->setDisabled(true);
                ui->RF_button->setPalette(Ui_SetColor(QPalette::ButtonText, Qt::white, QPalette::Button,Qt::darkGray));
#ifndef _ONPC_
                uart_fd = gpioTools::uart_init();
                if(uart_fd == -1)
                {
                    qDebug()<<"uart connect error!";
                }
                else
                {
                    qDebug()<<"uart connect success!";
                    notRsRead = new QSocketNotifier(uart_fd , QSocketNotifier::Read , this);
                    connect(notRsRead , SIGNAL(activated(int)) , this, SLOT(slot_Read_Event()));
                }
#endif
#ifndef __TSRF_TEST__
                gpioTools::uart_printf("%d,%d,%d,%d,%d\n",
                        data_in_index++,
                        sysParm->mRfDeviceFactors->tmDACValueToSetforCal,
                        sysParm->mRfDeviceFactors->tmReadADCVoltage,
                        sysParm->mRfDeviceFactors->tmReadADCCurrent,
                        sysParm-> mRfDeviceFactors->tmReadADCTemperatureA,
                        sysParm->mRfDeviceFactors->tmReadADCTemperatureB );


                qDebug("%d,%d,%d,%d,%d\n",
                        data_in_index++,
                        sysParm->mRfDeviceFactors->tmDACValueToSetforCal,
                        sysParm->mRfDeviceFactors->tmReadADCVoltage,
                        sysParm->mRfDeviceFactors->tmReadADCCurrent,
                        sysParm->mRfDeviceFactors->tmReadADCTemperatureA,
                        sysParm->mRfDeviceFactors->tmReadADCTemperatureB);



#endif
            }
            else
            {
                ui->RF_button->setEnabled(true);
                ui->Up_button->setEnabled(true);
                ui->Down_button->setEnabled(true);
                if(!sysParm->isDoTestforCal)
                {
                    ui->RF_button->setPalette(Ui_SetColor(QPalette::ButtonText, Qt::white, QPalette::Button,Qt::blue));
                }
                else
                {
                    ui->RF_button->setPalette(Ui_SetColor(QPalette::ButtonText, Qt::white, QPalette::Button,Qt::red));
                }
#ifndef _ONPC_
                gpioTools::uart_close();
                qDebug()<<"uart close!";
#endif
            }

            break;

        default:
            break;
    }
}

void calibration::slot_btnDirReleasedCal(int keyEvent)
{
    int32 DAC_Cal_Value = sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
    int32 V_DIV_Level = sysParm->mRfDeviceFactors->v_division_level;
    int32 I_DIV_Level = sysParm->mRfDeviceFactors->i_division_level;
    switch( keyEvent )
    {
        case BTNS::BTN_UP:
            switch(sysParm->mRfDeviceFactors->mode_click_state)
            {
                case CLICK_CALIBRATION_DAC_UNITS:
                    if(DAC_Cal_Value < 65535)
                    {
                         DAC_Cal_Value+=1;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_TENS:
                    if(DAC_Cal_Value < 65535)
                    {
                        DAC_Cal_Value+=10;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_HUNDREDS:
                    if(DAC_Cal_Value < 65535)
                    {
                        DAC_Cal_Value+=100;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_THOUSANDS:
                    if(DAC_Cal_Value < 65535)
                    {
                        DAC_Cal_Value+=1000;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_TENTHOUSANDS:
                    if(DAC_Cal_Value < 65535)
                    {
                        DAC_Cal_Value+=10000;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_V_UNITS:
                    if(V_DIV_Level < 999) //300
                    {
                       V_DIV_Level += 1;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_V_TENS:
                    if(V_DIV_Level < 999) //300
                    {
                       V_DIV_Level += 10;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_V_HUNDREDS:
                    if(V_DIV_Level < 999) //300
                    {
                       V_DIV_Level += 100;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_I_UNITS:
                    if(I_DIV_Level < 999) //200
                    {
                       I_DIV_Level += 1;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_I_TENS:
                    if(I_DIV_Level < 999) //200
                    {
                       I_DIV_Level += 10;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_I_HUNDREDS:
                    if(I_DIV_Level < 999) //200
                    {
                       I_DIV_Level += 100;
                    }
                    break;
                case CLICK_CALIBRATION_AVERAGE_COUNT:
                    if(sysParm->mRfDeviceFactors->average_count < 100)
                    {
                        sysParm->mRfDeviceFactors->average_count +=1;
                    }
                    break;
                default:
                    break;
            }
            if(DAC_Cal_Value >= 65535)
            {
                DAC_Cal_Value = 65535;
            }
            if(V_DIV_Level >= 999) //300
            {
                V_DIV_Level = 999;
            }
            if(I_DIV_Level >= 999) //200
            {
                I_DIV_Level = 999;
            }
            sysParm->mRfDeviceFactors->v_division_level = V_DIV_Level;
            sysParm->mRfDeviceFactors->i_division_level = I_DIV_Level;
            sysParm->mRfDeviceFactors->tmDACValueToSetforCal = DAC_Cal_Value;
            if((sysParm->isDoTestforCal) && (DAC_Cal_Value <=65535))
            {
                sysParm->mRfDeviceFactors->tmDACValueToSetLast = sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
            }

            //qDebug() << "DAC_Cal_Value = "<<DAC_Cal_Value;
            //qDebug() << sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
            break;

        case BTNS::BTN_DOWN:
            switch(sysParm->mRfDeviceFactors->mode_click_state)
            {
                case CLICK_CALIBRATION_DAC_UNITS:
                    if(DAC_Cal_Value >= 0)
                    {
                        DAC_Cal_Value-=1;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_TENS:
                    if(DAC_Cal_Value >= 0)
                    {
                        DAC_Cal_Value-=10;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_HUNDREDS:
                    if(DAC_Cal_Value >= 0)
                    {
                        DAC_Cal_Value-=100;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_THOUSANDS:
                    if(DAC_Cal_Value >= 0)
                    {
                        DAC_Cal_Value-=1000;
                    }
                    break;
                case CLICK_CALIBRATION_DAC_TENTHOUSANDS:
                    if(DAC_Cal_Value >= 0)
                    {
                        DAC_Cal_Value-=10000;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_V_UNITS:
                    if(V_DIV_Level > 0) //100
                    {
                       V_DIV_Level -= 1;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_V_TENS:
                    if(V_DIV_Level > 0) //100
                    {
                       V_DIV_Level -= 10;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_V_HUNDREDS:
                    if(V_DIV_Level > 100) //100
                    {
                       V_DIV_Level -= 100;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_I_UNITS:
                    if(I_DIV_Level > 0) //50
                    {
                       I_DIV_Level -= 1;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_I_TENS:
                    if(I_DIV_Level > 0) //50
                    {
                       I_DIV_Level -= 10;
                    }
                    break;
                case CLICK_CALIBRATION_DIVISION_I_HUNDREDS:
                    if(I_DIV_Level > 0) //50
                    {
                       I_DIV_Level -= 100;
                    }
                    break;
                case CLICK_CALIBRATION_AVERAGE_COUNT:
                    if(sysParm->mRfDeviceFactors->average_count > 1)
                    {
                        sysParm->mRfDeviceFactors->average_count -=1;
                    }
                    break;

                default :
                    break;
            }
            if(DAC_Cal_Value < 0)
            {
                DAC_Cal_Value = 0;
            }
            if(V_DIV_Level < 0) //100
            {
                V_DIV_Level = 0;
            }
            if(I_DIV_Level < 0) //50
            {
                I_DIV_Level = 0;
            }
            sysParm->mRfDeviceFactors->v_division_level = V_DIV_Level;
            sysParm->mRfDeviceFactors->i_division_level = I_DIV_Level;
            sysParm->mRfDeviceFactors->tmDACValueToSetforCal = DAC_Cal_Value;
            if((sysParm->isDoTestforCal) && (DAC_Cal_Value >= 0))
            {
                sysParm-> mRfDeviceFactors->tmDACValueToSetLast = sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
            }
           // qDebug() << sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
            break;

        default:
            break;
    }
}

void calibration::slot_Read_Event()
{
   // qDebug()<<"Read Data Ok";
    char *pdata;
    int length;

    int i = 0;
    uint16 ReadData[6];
#ifndef _ONPC_
    pdata = gpioTools::uart_binread(&length);
    qDebug()<<"Read Data = " <<pdata;
    char *ptr = strtok(pdata, ",");
    while(ptr != NULL)
    {
       // qDebug("%s",ptr);
        sscanf(ptr,"%d",ReadData+i);
        qDebug("ReadData[%d] = %d", i , *(ReadData+i));
        ptr = strtok(NULL,",");
        i ++;
    }

    sysParm->mRfDeviceFactors->tmDACValueToSetforCal = *(ReadData+0);
    sysParm->mRfDeviceFactors->tmReadADCVoltage = *(ReadData+1);
    sysParm->mRfDeviceFactors->tmReadADCCurrent = *(ReadData+2);
    sysParm->mRfDeviceFactors->rfVoltageMeasured = *(ReadData+3);
    sysParm->mRfDeviceFactors->rfCurrentMeasured = *(ReadData+4);
    sysParm->mRfDeviceFactors->rfImpedanceMeasured = *(ReadData+5);

#endif
#ifdef __TSRF_TEST__
#else
    char *pdata;
    int length;
    pdata = gpioTools::uart_binread(&length);
    memcpy(read_Data, pdata, length);
    read_Data_length = length;

    for(int i  = 0 ; i< length; i ++)
    {
        qDebug("[%d]=%02X", i, 0xFF & pdata[i]);
    }
    qDebug()<<"read_Data_length = "<<read_Data_length;

    emit sig_data_display();
#endif


}
/*
void calibration::slot_calibration_in()
{
    Show();
    IsPressed = false;
    sysParm->mRfDeviceFactors->tmDACValueToSetLast = 0;
    data_in_index = 0;
    DisplayTimer->start();
    Choose_Click_state(CLICK_NONE);
}
*/
void calibration::UI_Hide()
{
    ui->Back_button->hide();
    ui->DAC_label->hide();
    ui->DAC_tens_LCD->hide();
    ui->Down_button->hide();
    ui->I_label->hide();
    ui->I_LCD->hide();
    ui->Modelabel->hide();
    ui->RF_button->hide();
    ui->R_label->hide();
    ui->R_LCD->hide();
    ui->serial_label->hide();
    ui->SN_label->hide();
    ui->SW_label->hide();
    ui->sw_label->hide();
    ui->Up_button->hide();
    ui->USB_button->hide();
    ui->V_label->hide();
    ui->V_LCD->hide();
    ui->W_label->hide();
    ui->W_LCD->hide();
    ui->W_symbol_label->hide();
    ui->R_symbol_label->hide();
    ui->DAC_units_LCD->hide();
    ui->DAC_units_line->hide();
    ui->DAC_tens_LCD->hide();
    ui->DAC_tens_line->hide();
    ui->DAC_hundreds_LCD->hide();
    ui->DAC_hundreds_line->hide();
    ui->DAC_thousands_LCD->hide();
    ui->DAC_thousands_line->hide();
    ui->DAC_tenthousands_LCD->hide();
    ui->DAC_tenthousands_line->hide();
    ui->V_division_label->hide();
    ui->I_division_label->hide();
    ui->V_units_LCD->hide();
    ui->V_units_line->hide();
    ui->V_tens_LCD->hide();
    ui->V_tens_line->hide();
    ui->V_hundreds_LCD->hide();
    ui->V_hundreds_line->hide();
    ui->V_result_label->hide();
    ui->V_result_LCD->hide();
    ui->V_result_symbol_label->hide();
    ui->I_units_LCD->hide();
    ui->I_units_line->hide();
    ui->I_tens_LCD->hide();
    ui->I_tens_line->hide();
    ui->I_hundreds_LCD->hide();
    ui->I_hundreds_line->hide();
    ui->I_result_label->hide();
    ui->I_result_LCD->hide();
    ui->I_result_symbol_label->hide();

    ui->W7_button->hide();
    ui->W15_button->hide();
    ui->W25_button->hide();
    ui->W50_button->hide();

}

void calibration::UI_Show()
{
    ui->Back_button->show();
    ui->DAC_label->show();
    ui->DAC_tens_LCD->show();
    ui->Down_button->show();
    ui->I_label->show();
    ui->I_LCD->show();
    ui->Modelabel->show();
    ui->RF_button->show();
    ui->R_label->show();
    ui->R_LCD->show();
    ui->serial_label->show();
    ui->SN_label->show();
    ui->SW_label->show();
    ui->sw_label->show();
    ui->Up_button->show();
    ui->USB_button->show();
    ui->V_label->show();
    ui->V_LCD->show();
    ui->W_label->show();
    ui->W_LCD->show();
    ui->W_symbol_label->show();
    ui->R_symbol_label->show();
    ui->DAC_units_LCD->show();
    ui->DAC_units_line->show();
    ui->DAC_tens_LCD->show();
    ui->DAC_tens_line->show();
    ui->DAC_hundreds_LCD->show();
    ui->DAC_hundreds_line->show();
    ui->DAC_thousands_LCD->show();
    ui->DAC_thousands_line->show();
    ui->DAC_tenthousands_LCD->show();
    ui->DAC_tenthousands_line->show();
    ui->V_division_label->show();
    ui->I_division_label->show();
    ui->V_units_LCD->show();
    ui->V_units_line->show();
    ui->V_tens_LCD->show();
    ui->V_tens_line->show();
    ui->V_hundreds_LCD->show();
    ui->V_hundreds_line->show();
    ui->V_result_label->show();
    ui->V_result_LCD->show();
    ui->V_result_symbol_label->show();
    ui->I_units_LCD->show();
    ui->I_units_line->show();
    ui->I_tens_LCD->show();
    ui->I_tens_line->show();
    ui->I_hundreds_LCD->show();
    ui->I_hundreds_line->show();
    ui->I_result_label->show();
    ui->I_result_LCD->show();
    ui->I_result_symbol_label->show();

    ui->W7_button->show();
    ui->W15_button->show();
    ui->W25_button->show();
    ui->W50_button->show();

}
void calibration::Limited_Button()
{
    switch(sysParm->mRfDeviceFactors->mode_click_state)
    {
        case CLICK_NONE:
            ui->Up_button->setEnabled(false);
            ui->Down_button->setEnabled(false);

            break;

        case CLICK_CALIBRATION_DAC_TENS:
        /*
            if(sysParm->mRfDeviceFactors->tmDACValueToSetforCal >= 65000) //Max Level
            {
                ui->Up_button->setEnabled(false);
                ui->Down_button->setEnabled(true);
            }
            else if(sysParm->mRfDeviceFactors->tmDACValueToSetforCal <= 0)
            {
                 ui->Up_button->setEnabled(true);
                 ui->Down_button->setEnabled(false);
            }
            else
            {
                ui->Up_button->setEnabled(true);
                ui->Down_button->setEnabled(true);
            }
        */
            ui->Up_button->setEnabled(true);
            ui->Down_button->setEnabled(true);
            break;
        case CLICK_CALIBRATION_DAC_HUNDREDS:
        /*
            if(sysParm->mRfDeviceFactors->tmDACValueToSetforCal >= 65000) //Max Level
            {
                ui->Up_button->setEnabled(false);
                ui->Down_button->setEnabled(true);
            }
            else if(sysParm->mRfDeviceFactors->tmDACValueToSetforCal <= 100)
            {
                 ui->Up_button->setEnabled(true);
                 ui->Down_button->setEnabled(false);
            }
            else
            {
                ui->Up_button->setEnabled(true);
                ui->Down_button->setEnabled(true);
            }
        */
        ui->Up_button->setEnabled(true);
        ui->Down_button->setEnabled(true);
            break;
        default:
            break;
    }
}


void calibration::installFilter()
{
    ui->RF_button->installEventFilter(this);
    ui->Back_button->installEventFilter(this);
    ui->Next_button->installEventFilter(this);
    ui->USB_button->installEventFilter(this);
    ui->Up_button->installEventFilter(this);
    ui->Down_button->installEventFilter(this);
    ui->Modelabel->installEventFilter(this);
    ui->DAC_units_LCD->installEventFilter(this);
    ui->DAC_tens_LCD->installEventFilter(this);
    ui->DAC_hundreds_LCD->installEventFilter(this);
    ui->DAC_thousands_LCD->installEventFilter(this);
    ui->DAC_tenthousands_LCD->installEventFilter(this);
    ui->Aver_LCD->installEventFilter(this);
    ui->V_units_LCD->installEventFilter(this);
    ui->V_tens_LCD->installEventFilter(this);
    ui->V_hundreds_LCD->installEventFilter(this);
    ui->I_units_LCD->installEventFilter(this);
    ui->I_tens_LCD->installEventFilter(this);
    ui->I_hundreds_LCD->installEventFilter(this);
    ui->W7_button->installEventFilter(this);
    ui->W15_button->installEventFilter(this);
    ui->W25_button->installEventFilter(this);
    ui->W50_button->installEventFilter(this);
}

void calibration::slot_Button_Loop()
{
        if(PressedButton == ui->Back_button )
        {
            if(Button_Counting > 1) //10ms
            {
                //emit sig_calibration();
                if(sysParm->isDoTestforCal == true)
                {
                    emit sig_btnReleasedCal(BTNS::BTN_RFSTARTSTOP);
                }
                sysParm->mRfDeviceFactors->tmDACValueToSetLast =  0;
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal = 0;
                DisplayTimer->stop();
                //emit sig_setting();
                emit sig_ScreenIn_Setup();
                this->close();
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->Next_button )
        {
            if(Button_Counting > 1) //10ms
            {
                //emit sig_calibration();
                if(sysParm->isDoTestforCal == true)
                {
                    emit sig_btnReleasedCal(BTNS::BTN_RFSTARTSTOP);
                }
                sysParm->mRfDeviceFactors->tmDACValueToSetLast =  0;
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal = 0;
                DisplayTimer->stop();
                //emit sig_setting();
                emit sig_ScreenIn_Engineer(CALL_FROME_CALIBRATION);
                UI_Hide();
                //this->close();
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->RF_button && ui->RF_button->isEnabled()==true )
        {
            if(Button_Counting > 1) //10ms
            {
                emit sig_btnReleasedCal(BTNS::BTN_RFSTARTSTOP);
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->USB_button)
        {
            if(Button_Counting > 1) //10ms
            {
                IsUSBstate ^= true;
                emit sig_btnReleasedCal(BTNS::BTN_MODE);
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->W7_button)
        {
            if(Button_Counting > 1) //10ms
            {
                sysParm->mRfDeviceFactors->watt_select = WATT_7;
                qDebug("7w Button Click");
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->W15_button)
        {
            if(Button_Counting > 1) //10ms
            {
                sysParm->mRfDeviceFactors->watt_select = WATT_15;
                qDebug("15w Button Click");
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->W25_button)
        {
            if(Button_Counting > 1) //10ms
            {
                sysParm->mRfDeviceFactors->watt_select = WATT_25;
                qDebug("25w Button Click");
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->W50_button)
        {
            if(Button_Counting > 1) //10ms
            {
                sysParm->mRfDeviceFactors->watt_select = WATT_50;
                qDebug("50w Button Click");
                Button_Timer->stop();
            }
        }
        else if(PressedButton == ui->Up_button && ui->Up_button->isEnabled()==true)
        {
            if(Button_Counting > 3) //50ms
            {
                emit sig_btnDirReleasedCal(BTNS::BTN_UP);
                Button_Counting = 0;
            }
        }
        else if(PressedButton == ui->Down_button && ui->Down_button->isEnabled()==true)
        {
            if(Button_Counting > 3) //50ms
            {
                emit sig_btnDirReleasedCal(BTNS::BTN_DOWN);
                Button_Counting = 0;
            }
        }
        else if(LCDNumber == ui->DAC_units_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DAC_UNITS);
                //qDebug() << "Click DAC_units_LCD";
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->DAC_tens_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DAC_TENS);
                //qDebug() << "Click DAC_tens_LCD";
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->DAC_hundreds_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DAC_HUNDREDS);
                //qDebug() << "Click DAC_hundres_LCD";
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->DAC_thousands_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DAC_THOUSANDS);
                //qDebug() << "Click DAC_thousands_LCD";
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->DAC_tenthousands_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DAC_TENTHOUSANDS);
                //qDebug() << "Click DAC_tenthusands_LCD";
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->V_units_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DIVISION_V_UNITS);
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->V_tens_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DIVISION_V_TENS);
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->V_hundreds_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DIVISION_V_HUNDREDS);
                Button_Timer->stop();
            }
        }

        else if(LCDNumber == ui->I_units_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DIVISION_I_UNITS);
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->I_tens_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DIVISION_I_TENS);
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->I_hundreds_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_DIVISION_I_HUNDREDS);
                Button_Timer->stop();
            }
        }
        else if(LCDNumber == ui->Aver_LCD)
        {
            if(Button_Counting > 1) //10ms
            {
                Choose_Click_state(CLICK_CALIBRATION_AVERAGE_COUNT);
                Button_Timer->stop();
            }
        }


#if 0
        else if(Label == ui->Modelabel)
        {
            if(Button_Counting > 1) //10ms
            {
                /*
                emit sig_calibration_in();
                Hide();
                */

                //emit sig_engineer_in();
                DisplayTimer->stop();
                Button_Timer->stop();
                Hide();

            }
        }
#endif


        Button_Counting++; //10ms
}

bool calibration::eventFilter(QObject *target, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(IsPressed == false)
        {
            PressedButton = (QPushButton *)target;
            LCDNumber =(QLCDNumber *)target;
            Label = (QLabel *)target;
            IsPressed = true;
            Button_Counting = 0;
            Button_Timer->setInterval(10);
            Button_Timer->start();
        }
        if(Button_Counting > 3)
        {
            PressedButton = nullptr;
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        if(IsPressed == true)
        {
            PressedButton = nullptr;
            LCDNumber = nullptr;
            Label = nullptr;
            IsPressed = false;
            Button_Timer->stop();
        }
    }
    return QWidget::eventFilter(target, event);
}
