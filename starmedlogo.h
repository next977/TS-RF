#ifndef STARMEDLOGO_H
#define STARMEDLOGO_H

#include <QWidget>
#include <QMovie>
#include <QElapsedTimer>
#include <QTimer>
#include "gpiotools.h"
namespace Ui {
class starmedlogo;
}

class starmedlogo : public QWidget
{
    Q_OBJECT

public:
    bool eventFilter(QObject *target, QEvent *event);
    explicit starmedlogo(QWidget *parent = 0);
    ~starmedlogo();

private:
    Ui::starmedlogo *ui; 
    QElapsedTimer elapsedTime;
    QTimer *Display_Timer;
    QMovie *movie_gif;
    void SetConnect();
    void Setimage();
    void starmedlogo_hide();

    bool LCDBackLight_State;

private slots:
    void slot_Display();

signals:
    //void sig_Initailizing();
    void sig_Main_Screen();
};

#endif // STARMEDLOGO_H
