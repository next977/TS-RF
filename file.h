#ifndef FILE_H
#define FILE_H
#include "sysparm.h"
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDebug>

class SysParm;
class File_RW : public QObject
{
    Q_OBJECT

public:

    enum File_Type
    {
        TYPE_LOGDATA,
        TYPE_MRFA_TABLE,
        TYPE_TEMPERATURE,
        TYPE_CALIBRATION
    };


    SysParm *sysParm;
    explicit File_RW(QObject *parent = nullptr , SysParm *sysParm=nullptr);
    ~File_RW();

    QString filename;



    void FileWrite(uint16 Mode);
    bool FileRead(File_Type file_type);
    void FileOpen(File_Type file_type);
    void FileClose();

private:
    QString FileDate();
    uint16 filenumber;
signals:


};


#endif // FILE_H
