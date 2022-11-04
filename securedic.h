#ifndef SECUREDIC_H
#define SECUREDIC_H

#include <QObject>
#include "ds28e25_ds28e22_ds28e15.h"
#include "sha256_software.h"
#include "1wire_ds2465.h"
#include "gpiotools.h"
#include "sysparm.h"

class securedic : public QObject
{
    Q_OBJECT
public:
    SysParm *sysParm;
    explicit securedic(QObject *parent = nullptr , SysParm *sysParm=nullptr);
    ~securedic();

    int Find_DS28E25_DS28E22_DS28E15(int *num_pages);
    //void msDelay(int len);
    int dprintf(char *format, ...);
    void getRandom(uchar* buffer, int cnt);

signals:

public slots:
    void slot_SecuredIC_Start();
};

#endif // SECUREDIC_H
