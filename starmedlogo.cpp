#include "starmedlogo.h"
#include "ui_starmedlogo.h"

starmedlogo::starmedlogo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::starmedlogo)
{
    ui->setupUi(this);
    ui->starmed_logo_image->installEventFilter(this);


    Display_Timer = new QTimer(this);
    movie_gif = new QMovie(":/Images/booting.gif");
    Display_Timer->start(10);
    elapsedTime.start();

    LCDBackLight_State = false;
    Setimage();
    SetConnect();

    qDebug("logoStart");
}

starmedlogo::~starmedlogo()
{
    delete ui;
}
void starmedlogo::SetConnect()
{
    connect(Display_Timer, SIGNAL(timeout()), this, SLOT(slot_Display()));

}
void starmedlogo::slot_Display()
{
    if(elapsedTime.elapsed()/1000 >= 3 )
    {
        qDebug("STARmed logo Screen close");
        Display_Timer->stop();
        movie_gif->stop();
        emit sig_Main_Screen();
        this->close();
    }
    else if(elapsedTime.elapsed()/100 >= 3 && LCDBackLight_State == false )
    {
        LCDBackLight_State = true;
#ifndef _ONPC_
        gpioTools::LCDBackLight_ON();
#endif
    }

}

void starmedlogo::Setimage()
{
    //QMovie *movie_gif = new QMovie(":/Images/original_7.gif");
    //movie_gif->start();
    movie_gif->start();
    ui->starmed_logo_image->setMovie(movie_gif);
    ui->starmed_logo_image->setScaledContents(true);
}

bool starmedlogo::eventFilter(QObject *target, QEvent *event)
{
    if ((target == ui->starmed_logo_image) && (event->type() == QEvent::MouseButtonPress))
    {
        qDebug("starmedlogo_hide");
        //emit sig_Initailizing();
        Display_Timer->stop();
        movie_gif->stop();
        emit sig_Main_Screen();
        this->close();
    }
    return QWidget::eventFilter(target, event);
}
