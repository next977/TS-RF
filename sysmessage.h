#ifndef SYSMESSAGE_H
#define SYSMESSAGE_H

#include <QDialog>
#include <QPainter>
#include <QFontDatabase>
#include <QDebug>
#include <QFont>
#include <QTimer>
#include "sysparm.h"

namespace Ui {
class sysmessage;
}

class sysmessage : public QDialog
{
    Q_OBJECT

public:
    void setIDstarmed(int i) {id_starmed = i; }
    explicit sysmessage(QWidget *parent = 0);
    ~sysmessage();

protected:
    //void paintEvent(QPaintEvent *event);

private:
    Ui::sysmessage *ui;
    void SetFont();
    void Setimage();
    int id_starmed;

public slots:
   void slot_alarm_code(Alarm_Code code);
   void slot_error_code(Error_Code code);

private slots:
    void slot_sysmessage_close();
signals:
    void sig_Main_Screen();


};

#endif // SYSMESSAGE_H
