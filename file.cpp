#include <QDateTime>
#include <QDir>
#include "file.h"
#define UTC_TIME_ASIA_SEOUL    (9 * 60 * 60)   // 9h
File_RW::File_RW(QObject *parent, SysParm *sysParm) :
    QObject(parent)
{
    this->sysParm = sysParm;
    filenumber = 0;

}

File_RW::~File_RW()
{
}

QString File_RW::FileDate()
{
    QString file_date,file_time,file_datetime;
    file_date.sprintf("%02d-%02d-%02d", sysParm->mRfDeviceFactors->Date_year ,sysParm->mRfDeviceFactors->Date_month
                      ,sysParm->mRfDeviceFactors->Date_day);
    file_time.sprintf("%02d.%02d.%02d", sysParm->mRfDeviceFactors->Time_hour , sysParm->mRfDeviceFactors->Time_min
                      ,sysParm->mRfDeviceFactors->Time_sec); //"%02d:%02d:%02d"
    file_datetime = file_date +'_'+ file_time;
    return file_datetime;
}

void File_RW::FileOpen(File_Type file_type)
{
#ifndef __TSRF_TEST__
    QString datetime_format = "yyyy-MM-dd_HH:mm:ss";
    QDateTime file_date = QDateTime::currentDateTime();
    QString filedate = file_date.toString(datetime_format);
#endif
    if(file_type == TYPE_LOGDATA)
    {
        QString dirPath = "LogData";
#ifndef _ONPC_
        QDir dir("/app/app"); //STRIKER Release path
#else
        QDir dir; //STRIKER DEBUG path
#endif
        filenumber ++;
        filename.sprintf("file_%d_",filenumber);

        filename = filename + FileDate();
        //dir
        dir.mkpath(dirPath);
        filename = dir.absolutePath() + '/' + dirPath +'/' + filename;
    }
    else if(file_type == TYPE_MRFA_TABLE)
    {

#ifndef _ONPC_
        filename = "/app/app/mrfa_table/test_mrfa_rf_table.c";
#else
        filename = "test_mrfa_rf_table.c";
#endif
    }
    else if(file_type == TYPE_TEMPERATURE)
    {
        filename = "/sys/class/thermal/thermal_zone0/temp";
    }
    else if(file_type == TYPE_CALIBRATION)
    {
        QString dirPath = "Calibration";
#ifndef _ONPC_
        QDir dir("/app/app"); //STRIKER Release path
#else
        QDir dir; //TSRF DEBUG path
#endif
        filename.sprintf("Calibration_Settingfile");
        //dir
        qDebug() << "File Type : CALIBRATION ";
        dir.mkpath(dirPath);
    }
}

void File_RW::FileClose()
{
    //filenumber = 0;
}

void File_RW::FileWrite(uint16 Mode)
{

    QFile file(filename);
    QTextStream Write(&file);
    if(!file.open(QFile::WriteOnly|QFile::Append|QFile::Text)) //real file open
    {
        qDebug("File_Open_Fail_WriteOnly");
        return ;
    }
    else
    {
        qDebug("File_Open_Success_WriteOnly");
    }
    for(int i  = 0;  i < sysParm->mRfDeviceFactors->rf_VM.size() ; i++)
    {
        switch(sysParm->mRfDeviceFactors->rf_State.at(i))
        {
            case 1:
#if 0
                    /*
                    if(Mode == 0)
                    {
                        Write<< "Puncture Mode \n" ;
                        Write<< "Puncture Time : " << sysParm->tmRFCogluationTime_Puncture/10 << '.'
                                                    << sysParm->tmRFCogluationTime_Puncture%10 <<" s \n";
                        Write<< "Continue Time : " << sysParm->tmRFCogluationTime_Continue/10 << '.'
                                                    << sysParm->tmRFCogluationTime_Continue%10 <<" s \n";
                    }
                    else if(Mode == 1)
                    {
                        Write<< "Pulse Mode \n" ;
                        Write<< "     Pulse Time : " << sysParm->tmRFCogluationTime_Pulse <<" ms \n";
                        Write<< "     Idle  Time : " << sysParm->tmRFCogluationTime_Idle <<" ms \n";
                        Write<< "Pulse Mode Time : " << sysParm->tmRFCogluationTime_PulseMode <<" ms \n";
                        Write<< "          Count : " << sysParm->mRfDeviceFactors->rfPulseCount <<" \n";
                    }
                    Write<< "      End Time : " << sysParm->tmElapsedTime/10
                         << " s \n";

                    Write<< "\n<------------------Impedance Check Start--------------->\n\n" ;
                    Write<< "count, V, I, R, PM, DAC  \n";

                    */
#endif
                    Write<< "\n<------------------------ Settings Informatio--------------------->\n\n" ;

#ifdef __FEATURE_TSRF__
#else
                    Write<< "Settings: SartTime DeviceID Time Mode Volume Brightness StartupTime StartupMode Language\n" ;
                    Write<< "Settings: " << SartTime << DeviceID << Time << Mode << Volume << Brightness << StartupTime << StartupMode << Language << "\n";
                    Write<< "Info: " << Date_Time << ID << Description << "\n";
                    Write<< "Alarm: " << Date_Time << ID << Description << "\n";
                    Write<< "Error: " << Date_Time << ID << Description << "\n";

                    Write<< "\n<------------------------ Data Information--------------------->\n\n" ;
                    Write<< "Data: Count Current Voltage Power Impedance Dutycycle\n";
#endif
                    break;
            case 2:
#if 0
                    Write<< "\n<------------------------Mode Start-------------------->\n\n" ;
                    Write<< "count, V, I, R, PM, DAC  \n";
#endif
#ifdef __FEATURE_TSRF__
#else
                    Write<< "Data: " << Count << Current << Voltage << Power << Impedance << Dutycycle << "\n";
#endif
                    break;
            case 3:
#if 0
                    Write<< "\n<------------------------Ready------------------------->\n\n" ;
                    Write<< "count, V, I, R, PM, DAC  \n";
#endif
                    break;
            case 4:
#if 0
                    Write<< "\n<------------------Puncture continue Start-------------->\n\n" ;
                    Write<< "count, V, I, R, PM, DAC  \n";
#endif
                    break;
            default :
#if 0
                    /*
                    Write << sysParm->mRfDeviceFactors->rf_Count.at(i) << " , ";
                    Write << sysParm->mRfDeviceFactors->rf_VM.at(i) << " , ";
                    Write << sysParm->mRfDeviceFactors->rf_CM.at(i) << " , ";
                    Write << sysParm->mRfDeviceFactors->rf_IM.at(i) << " , ";
                    Write << sysParm->mRfDeviceFactors->rf_PM.at(i) << " , ";
                    Write << sysParm->mRfDeviceFactors->rf_DAC.at(i) << "\n";
                    */
#endif
                    break;
        }
    }


    file.close();
}

bool File_RW::FileRead(File_Type file_type)
{
     QString Line_str, Head, Body, Tail, Fileds_Value, Read_Temp_Str;
     QStringList fields;

     QFile file(filename);
     QTextStream Read(&file);

     bool readLine_State = false;
     qDebug()<<"filename = "<<filename;

     if(file_type == TYPE_MRFA_TABLE)
     {
         if(!file.open(QFile::ReadOnly|QFile::Text)) //real file open
         {
            qDebug() << "Mrfa_table/Test mrfa open fail";
            return false;
         }
         else
         {
             qDebug() << "Test Mrfa Open_ReadOnly_Success = " <<  filename;
         }
         while(!Read.atEnd())
         {
             Line_str = Read.readLine();
             if(readLine_State == false)
             {
                 if(Line_str.contains("TABLE_DEVICE_ID",Qt::CaseInsensitive))
                 {
                     fields = Line_str.split("=");
                     Head = fields.value(0);
                     Tail = fields.value(1);
                     sysParm->File_TABLE_DEVICE_ID = Tail.toInt();
                     qDebug()<< "sysParm->File_TABLE_DEVICE_ID = " << sysParm->File_TABLE_DEVICE_ID;

                 }
                 else if(Line_str.contains("TABLE_STEP_VOLTAGE",Qt::CaseInsensitive))
                 {
                     fields = Line_str.split("=");
                     Head = fields.value(0);
                     Tail = fields.value(1);
                     sysParm->File_TABLE_STEP_VOLTAGE = Tail.toInt();
                     qDebug()<< "sysParm->File_TABLE_STEP_VOLTAGE = " << sysParm->File_TABLE_STEP_VOLTAGE;

                 }
                 else if(Line_str.contains("TABLE_STEP_CURRENT",Qt::CaseInsensitive))
                 {
                     fields = Line_str.split("=");
                     Head = fields.value(0);
                     Tail = fields.value(1);
                     sysParm->File_TABLE_STEP_CURRENT = Tail.toInt();
                     qDebug()<< "File_TABLE_STEP_CURRENT = " << sysParm->File_TABLE_STEP_CURRENT;

                 }
                 else if(Line_str.contains("TABLE_STEP_RFPOWER_REF",Qt::CaseInsensitive))
                 {
                     fields = Line_str.split("=");
                     Head = fields.value(0);
                     Tail = fields.value(1);
                     sysParm->File_TABLE_STEP_RFPOWER_REF = Tail.toInt();
                     qDebug()<< "sysParm->File_TABLE_STEP_RFPOWER_REF = " << sysParm->File_TABLE_STEP_RFPOWER_REF;
                     readLine_State = true;
                 }
             }
             else
             {
                 if(Line_str.contains("TransTableRFPowerToDAC",Qt::CaseInsensitive))
                 {
                    Line_str = Read.readLine();
                    for(int i =0 ; i <19 ; i ++)
                    {
                        Line_str = Read.readLine();
                        //qDebug() << "MRFA_Read_readLine = " << Line_str;
                        fields = Line_str.split("},{");
                        Head = fields.value(0);
                        Body = fields.value(1);
                        Tail = fields.value(2);
                        fields = Head.split(',');
                        Fileds_Value = fields.value(1);
                        //sysParm->FileTransTableRFPowerToDAC[i][1].tmRealValue=Fileds_Value.toInt();
                        sysParm->FileTransTableRFPowerToDAC[i][0].tmDigitalValue=Fileds_Value.toInt();
                        Head = fields.value(0);
                        fields = Head.split("{{");
                        Fileds_Value = fields.value(1);
                        sysParm->FileTransTableRFPowerToDAC[i][0].tmRealValue=Fileds_Value.toInt();
                        //qDebug() << "TransTableRFPowerToDAC[i][0] = " << sysParm->FileTransTableRFPowerToDAC[i][0].tmDigitalValue;
                        //qDebug() << "TransTableRFPowerToDAC[i][0] = " << sysParm->FileTransTableRFPowerToDAC[i][0].tmRealValue;
                    }
                 }
                 else if(Line_str.contains("TransTableRFVoltage",Qt::CaseInsensitive))
                 {
                    Line_str = Read.readLine();
                    for(int i =0 ; i <19 ; i ++)
                    {
                        Line_str = Read.readLine();
                        //qDebug() << "MRFA_Read_readLine = " << Line_str;
                        fields = Line_str.split("},{");
                        Head = fields.value(0);
                        Body = fields.value(1);
                        Tail = fields.value(2);
                        fields = Head.split(',');
                        Fileds_Value = fields.value(1);
                        //sysParm->FileTransTableRFVoltage[i][1].tmRealValue=Fileds_Value.toInt();
                        sysParm->FileTransTableRFVoltage[i][0].tmDigitalValue=Fileds_Value.toInt();
                        Head = fields.value(0);
                        fields = Head.split("{{");
                        Fileds_Value = fields.value(1);
                        sysParm->FileTransTableRFVoltage[i][0].tmRealValue=Fileds_Value.toInt();
                        //qDebug() << "TransTableRFVoltage[i][0] = " << sysParm->FileTransTableRFVoltage[i][0].tmDigitalValue;
                       // qDebug() << "TransTableRFVoltage[i][0] = " << sysParm->FileTransTableRFVoltage[i][0].tmRealValue;
                    }
                 }
                 else if(Line_str.contains("TransTableRFCurrent",Qt::CaseInsensitive))
                 {
                    Line_str = Read.readLine();
                    for(int i =0 ; i <19 ; i ++)
                    {
                        Line_str = Read.readLine();
                        //qDebug() << "MRFA_Read_readLine = " << Line_str;
                        fields = Line_str.split("},{");
                        Head = fields.value(0);
                        Body = fields.value(1);
                        Tail = fields.value(2);
                        fields = Head.split(',');
                        Fileds_Value = fields.value(1);
                       // sysParm->FileTransTableRFCurrent[i][1].tmRealValue=Fileds_Value.toInt();
                        sysParm->FileTransTableRFCurrent[i][0].tmDigitalValue=Fileds_Value.toInt();
                        Head = fields.value(0);
                        fields = Head.split("{{");
                        Fileds_Value = fields.value(1);
                        sysParm->FileTransTableRFCurrent[i][0].tmRealValue=Fileds_Value.toInt();
                       // qDebug() << "TransTableRFCurrent[i][0] = " << sysParm->FileTransTableRFCurrent[i][0].tmDigitalValue;
                       // qDebug() << "TransTableRFCurrent[i][0] = " << sysParm->FileTransTableRFCurrent[i][0].tmRealValue;
                    }
                 }
             }
         }
     }
     else if(file_type == TYPE_TEMPERATURE)
     {
         if(!file.open(QFile::ReadOnly|QFile::Text)) //real file open
         {
             //qDebug() << "File_Open_ReadOnly_Fail = " << filename;
             return false;
         }
         else
         {
             //qDebug() << "File_Open_ReadOnly_Success = " <<  filename;
         }

         Read_Temp_Str = Read.readLine();
         //sysParm->mRfDeviceFactors->cpu_temperature = Read_Temp_Str.toUInt();
     }

    file.close();
    return true;
}
