#ifndef USBOTG_H
#define USBOTG_H

#include <QObject>
#include <QWidget>
#include <QCoreApplication>
#include <QDebug>
#include <QLibrary>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QSerialPort>
//#include <QFtp>
#include <curl/curl.h>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkAccessManager>

#include <stdio.h>
#include <unistd.h>
//for network
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/route.h>

#include "include/usbg.h"
#include <termio.h>
#include <fcntl.h>


#define GADGET_SERIAL       "g_serial"
#define GADGET_ETH          "g_ether"
#define GADGET_ETH_DEF_IP   "192.168.0.100"

#define     __USING_QSerial__   0
#define     __USING_FILE__      1
#define     __USING_QFILE__     2

#define     DATA_ROUTINE      __USING_QSerial__// __USING_FILE__//__USING_QFILE__// __USING_QSerial__//__USING_QFILE__


class usbotg : public QObject
{
    Q_OBJECT
public:
    explicit usbotg(QObject *parent = 0);

private :
    void LibLoading(QString LibPath, QLibrary* LibObj);
    void LinkingFp();
    void priErr(usbg_error err);
    int set_ip(char *iface_name, const char *ip_addr);
    int init_SerialGadget(usbg_state *uStat, usbg_gadget *gadget, usbg_config *c);
    int init_EthGadget(usbg_state *uStat, usbg_gadget *gadget, usbg_config *c1, usbg_config *c2);
    int uart_init();
    void set_SerPortOpen(QSerialPort* pSP);
    void set_SerGadget(bool flag, QSerialPort* pSP, usbg_gadget* ug);
    void set_EthGadget(bool flag, QString strIP);
    QString get_SerData();
    QStringList parse_CmdData(QString srcData);
    QString get_FileFromHost(QString HostIp, QString FileName);



signals:

public slots:
};

#endif // USBOTG_H
