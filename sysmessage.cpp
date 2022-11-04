#include "sysmessage.h"
#include "ui_sysmessage.h"

sysmessage::sysmessage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sysmessage)
{
    ui->setupUi(this);
    int id = QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf");
    this->setIDstarmed(id);
    setWindowFlags(Qt::FramelessWindowHint); //위젯이나 다이얼로그의 테두리 없애기(타이틀바도 없어짐)
    Setimage();
    SetFont();
}

sysmessage::~sysmessage()
{
    delete ui;
}

void sysmessage::Setimage()
{
    //ui->Danger_symbol->setPixmap(QPixmap(":/Images/danger.png"));
    //ui->Danger_symbol->setScaledContents(true);
    //ui->Action_symbol->setPixmap(QPixmap(":/Images/action.png"));
    //ui->Action_symbol->setScaledContents(true);
}

void sysmessage::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont title_font(family,30, QFont::Bold);
    QFont data_font(family,30, QFont::Bold);
    //QFont Action_label_font(family,15);
    //QFont main_font(family,25);
    ui->code_title_label->setFont(title_font);
    ui->code_data_label->setFont(data_font);
    ui->Message_title_label->setFont(title_font);
    ui->Message_data_label->setFont(data_font);
    //ui->Action_label->setFont(label_font);
    //ui->Action_No_label->setFont(Action_label_font);
}

void sysmessage::slot_alarm_code(Alarm_Code code)
{
    ui->screen_label->setPixmap(QPixmap(":/Images/alarm.png"));
    ui->screen_label->setScaledContents(true);
    switch(code)
    {

    case ALARM_CODE_000: //>= 6000ohm
        ui->code_data_label->setText("000");
        ui->Message_data_label->setText("High Impedance");
        //ui->Code_No_label->setText("003");
        //ui->System_label->setText("Temperature Over");
        //ui->Action_No_label->setText("Please lower the temperature of the electrode");
        //ui->Action_No_label->setText("Please check temperature set point");
        break;
    case ALARM_CODE_001: //<= 100ohm
        ui->code_data_label->setText("001");
        ui->Message_data_label->setText("Low Impedance");
        //ui->Code_No_label->setText("001");
        //ui->System_label->setText("Impedance High");
        //ui->Action_No_label->setText("Please check the connection of electrode");
        break;
    case ALARM_CODE_002: //>= 6000ohm
        ui->code_data_label->setText("002");
        ui->Message_data_label->setText("Check Ground Pad");
        //ui->Code_No_label->setText("002");
        //ui->System_label->setText("Impedance Low");
        //ui->Action_No_label->setText("Please check the connection of electrode");
        break;
    case ALARM_CODE_003:
        ui->code_data_label->setText("003");
        ui->Message_data_label->setText("Check All Ground Pad");
        //ui->Code_No_label->setText("002");
        //ui->System_label->setText("Impedance Low");
        //ui->Action_No_label->setText("Please check the connection of electrode");
        break;

    default:
        break;
    }
    this->show();
    QTimer::singleShot(2000,this,SLOT(slot_sysmessage_close()));
}

void sysmessage::slot_error_code(Error_Code code)
{
    ui->screen_label->setPixmap(QPixmap(":/Images/error.png"));
    ui->screen_label->setScaledContents(true);
    switch(code)
    {

    case ERROR_CODE_000:
        ui->code_data_label->setText("000");
        //ui->Code_No_label->setText("003");
        //ui->System_label->setText("Temperature Over");
        //ui->Action_No_label->setText("Please lower the temperature of the electrode");
        //ui->Action_No_label->setText("Please check temperature set point");
        break;
    case ERROR_CODE_001:
        ui->code_data_label->setText("001");
        //ui->Code_No_label->setText("001");
        //ui->System_label->setText("Impedance High");
        //ui->Action_No_label->setText("Please check the connection of electrode");
        break;
    case ERROR_CODE_002:
        ui->code_data_label->setText("002");
        //ui->Code_No_label->setText("002");
        //ui->System_label->setText("Impedance Low");
        //ui->Action_No_label->setText("Please check the connection of electrode");
        break;
    case ERROR_CODE_003:
        ui->code_data_label->setText("003");
        //ui->Code_No_label->setText("002");
        //ui->System_label->setText("Impedance Low");
        //ui->Action_No_label->setText("Please check the connection of electrode");
        break;
    default:
        //ui->Code_No_label->setText("000");
        //ui->System_label->setText("None");
        //ui->Action_No_label->setText("None");
        break;
    }
    this->show();
    QTimer::singleShot(2000,this,SLOT(slot_sysmessage_close()));
}


void sysmessage::slot_sysmessage_close()
{
    this->close();
    emit sig_Main_Screen();
}

/*
void sysmessage::paintEvent(QPaintEvent *event) //위젯이나 다이얼로그의 테두리 그리기
{
    QPainter painter(this);
    //인자 설명 - 시작좌표x,시작좌표y,현재화면크기(길이),현재화면크기(높이)  0,0은 가장자리의 곡선설정
    painter.drawRoundedRect(0,0,width()-1,height()-1,0,0);
    QDialog::paintEvent(event);
}
*/
