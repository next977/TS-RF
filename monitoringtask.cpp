#include "monitoringtask.h"
#include <vector>
#include <QTimer>
#include "mrfa_rf_table.h"
#include "gpiotools.h"

#define __INTERVAL_MONITORING__ 0.01   //30ms, 33 cycle
#define __INTERVAL_TEST__ false
#define DATA_AVERAGE_COUNT 14

#if defined(__GNUC__) || defined(__HP_aCC) || defined(__clang__)
    #define INLINE inline __attribute__((always_inline))
#else
    #define INLINE __forceinline
#endif


using namespace std;

INLINE unsigned int fast_upper_bound2(const vector<uint16>& vec, uint16 value)
{
    unsigned int m_len = vec.size()/2;
    unsigned int low = 0;
    unsigned int high = m_len;

    while (high - low > 1) {
        unsigned int probe = (low + high) / 2;
        unsigned int v = vec[probe];
        if (v > value)
            high = probe;
        else
            low = probe;
    }

    if (high == m_len)
        return m_len;
    else
        return high;
}

INLINE unsigned int fast_lower_bound2(const vector<uint16>& vec, uint16 value)
{
    unsigned int m_len = vec.size()/2;
    unsigned int low = 0;
    unsigned int high = m_len;

    while (high - low > 1) {
        unsigned int probe = (low + high) / 2;
        unsigned int v = vec[probe];
        if (v >= value)
            high = probe;
        else
            low = probe;
    }

    if (high == m_len)
        return m_len;
    else
        return high;
}



MonitoringTask::MonitoringTask(QObject *_parent, SysParm *_sysParm) : QObject(_parent)
{
#if __INTERVAL_TEST__
    QTimer *checkpersec = new QTimer(this);
    counter = 0;
    connect(checkpersec, SIGNAL(timeout()), this, SLOT(slot_dispCounter()));
    checkpersec->start(1000);
#endif
    this->sysParm = _sysParm;

}

MonitoringTask::~MonitoringTask()
{
    if(timer->isActive())
    {
        timer->stop();
    }
    delete timer;
}

void MonitoringTask::RunTimer()
{
 
    timer = new QTimer();
    timer->setInterval(30);
    timer->start();

#if __INTERVAL_TEST__
    connect(timer, SIGNAL(timeout()), this, SLOT(run()));
#else
    connect(timer, SIGNAL(timeout()), this, SLOT(GetAmplifierParameters()));
#endif
 
}

void MonitoringTask::GetAmplifierParameters()
{
    int32 i,j;
    uint32 data_sum;
    uint16 tmp;
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;

    i = 0;
    /*
    data_sum =0;
    for( j = 0; j < DATA_AVERAGE_COUNT; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x05);   //CH1

        data_sum += MedianFilter(i, 0 , tmp);
    }
    mRfDeviceFactors->tmReadADCVoltage = static_cast<uint16>(data_sum/DATA_AVERAGE_COUNT);
    */
    /*
    data_sum =0;
    for( j = 0; j < DATA_AVERAGE_COUNT; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x01);   //CH0

        data_sum += MedianFilter(i, 1 , tmp);
    }
    mRfDeviceFactors->tmReadADCCurrent = static_cast<uint16>(data_sum/DATA_AVERAGE_COUNT);
    */


    data_sum =0;
    mRfDeviceFactors->tmReadADCVoltage =  gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x05);   //CH1
    for( j = 0; j < mRfDeviceFactors->average_count; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x05);   //CH1

        data_sum += MedianFilter(i, 0 , tmp);
    }
    mRfDeviceFactors->tmAverReadADCVoltage =  static_cast<uint16>(data_sum/mRfDeviceFactors->average_count);


    data_sum =0;
    mRfDeviceFactors->tmReadADCCurrent = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x01);   //CH0;
    for( j = 0; j < mRfDeviceFactors->average_count; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x01);   //CH0

        data_sum += MedianFilter(i, 1 , tmp);
    }
    mRfDeviceFactors->tmAverReadADCCurrent = static_cast<uint16>(data_sum/mRfDeviceFactors->average_count);


    data_sum =0;
    for( j = 0; j < DATA_AVERAGE_COUNT; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x02); //CH0
        data_sum += MedianFilter(i, 2 , tmp);
    }
    mRfDeviceFactors->tmReadADCTemperatureA = static_cast<uint16>(data_sum/DATA_AVERAGE_COUNT);
    //qDebug() << "mRfDeviceFactors->tmReadADCTemperatureA = " <<mRfDeviceFactors->tmReadADCTemperatureA;
    //gpioTools::msDelay(100);
    MRFA_TranslateCalculate();
    MRFA_TranslateRFTable();
}


void MonitoringTask::MRFA_TranslateCalculate()
{
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;
    uint32 j;

    uint16 rfa,rfb;
    uint16 div_factor;

    uint16 rfCurrentMeasured, rfVoltageMeasured, rfPowerMeasured, rfTemperatureMeasuredA,rfTemperatureMeasuredB;
    uint16 V_Real= mRfDeviceFactors->tmAverReadADCVoltage;//mRfDeviceFactors->tmReadADCVoltage;
    uint16 I_Real= mRfDeviceFactors->tmAverReadADCCurrent;//mRfDeviceFactors->tmReadADCCurrent;
    uint16 V_div_level = mRfDeviceFactors->v_division_level; //210
    uint16 I_div_level = mRfDeviceFactors->i_division_level; //110


    rfTemperatureMeasuredA = mRfDeviceFactors->tmReadADCTemperatureA;
    mRfDeviceFactors->rfTemperatureMeasuredA = rfTemperatureMeasuredA;

    if(V_Real > 0 && I_Real > 0)
    {
        double V_div_value = static_cast<double>(V_Real)/V_div_level;
        double I_div_value = static_cast<double>(I_Real)/I_div_level;
        double Watt = V_div_value * (I_div_value/1000);
        double Impedance = V_div_value / (I_div_value/1000);

        mRfDeviceFactors->rfPowerMeasured = static_cast<uint16>(Watt);
        mRfDeviceFactors->rfCurrentMeasured = static_cast<uint16>(I_div_value);
        mRfDeviceFactors->rfVoltageMeasured = static_cast<uint16>(V_div_value);
        mRfDeviceFactors->rfImpedanceMeasured = static_cast<uint16>(Impedance);

        if( mRfDeviceFactors->tmDACValueToSetLast == 0)//may be channel interference
        {
            mRfDeviceFactors->rfImpedanceMeasured = 9990;
            mRfDeviceFactors->rfTemperatureMeasuredA = 0;
            mRfDeviceFactors->rfPowerMeasured = 0;
            mRfDeviceFactors->rfCurrentMeasured = 0;
        }
    }
    else
    {
        mRfDeviceFactors->rfPowerMeasured = 0;
        mRfDeviceFactors->rfCurrentMeasured = 0;
        mRfDeviceFactors->rfVoltageMeasured = 0;
        mRfDeviceFactors->rfTemperatureMeasuredA = 0;
        mRfDeviceFactors->rfImpedanceMeasured = 9990;
    }
}

void MonitoringTask::MRFA_TranslateRFTable()
{
    uint32 j;

    uint16 rfa,rfb;
    uint16 div_factor;

    uint16 rfImpedanceTableMeasured7, rfImpedanceTableMeasured15, rfImpedanceTableMeasured25,  rfImpedanceTableMeasured50;
    uint16 rfDACTableMeasured7, rfDACTableMeasured15, rfDACTableMeasured25, rfDACTableMeasured50;
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;
    //////////////////////////////////////////7w//////////////////////////////////////////////////
    if(mRfDeviceFactors->watt_select == WATT_7)
    {
        rfa = mRfDeviceFactors->rfImpedanceMeasured;
        j = fast_upper_bound2(sysParm->vImp_Correct7, rfa);


        if(j == TABLE_STEP_IMP_CORRECT)
        {
            rfb = sysParm->vImp_Correct7.at(TABLE_STEP_IMP_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vImp_Correct7.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vImp_Correct7.at(j-1));
            rfb = ( sysParm->vImp_Correct7.at(TABLE_STEP_IMP_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vImp_Correct7.at(j-1) ) * 100 ) /
                         (	sysParm->vImp_Correct7.at(j) - sysParm->vImp_Correct7.at(j-1) );

            rfb = sysParm->vImp_Correct7.at(TABLE_STEP_IMP_CORRECT + j-1) +
                                     ( ( sysParm->vImp_Correct7.at(TABLE_STEP_IMP_CORRECT + j) - sysParm->vImp_Correct7.at(TABLE_STEP_IMP_CORRECT + j-1))* div_factor ) /100;
        }
        rfImpedanceTableMeasured7 = rfb;


        rfa = rfImpedanceTableMeasured7;
        j = fast_upper_bound2(sysParm->vDAC_Correct7, rfa);
        if(j == TABLE_STEP_DAC_CORRECT)
        {
            rfb = sysParm->vDAC_Correct7.at(TABLE_STEP_DAC_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vDAC_Correct7.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vDAC_Correct7.at(j-1));
            rfb = ( sysParm->vDAC_Correct7.at(TABLE_STEP_DAC_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vDAC_Correct7.at(j-1) ) * 100 ) /
                         (	sysParm->vDAC_Correct7.at(j) - sysParm->vDAC_Correct7.at(j-1) );

            rfb = sysParm->vDAC_Correct7.at(TABLE_STEP_DAC_CORRECT + j-1) +
                                     ( ( sysParm->vDAC_Correct7.at(TABLE_STEP_DAC_CORRECT + j) - sysParm->vDAC_Correct7.at(TABLE_STEP_DAC_CORRECT + j-1))* div_factor ) /100;
        }
        rfDACTableMeasured7 = rfb;

    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////15w//////////////////////////////////////////////////
    else if(mRfDeviceFactors->watt_select == WATT_15)
    {
        rfa = mRfDeviceFactors->rfImpedanceMeasured;
        j = fast_upper_bound2(sysParm->vImp_Correct15, rfa);


        if(j == TABLE_STEP_IMP_CORRECT)
        {
            rfb = sysParm->vImp_Correct15.at(TABLE_STEP_IMP_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vImp_Correct15.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vImp_Correct15.at(j-1));
            rfb = ( sysParm->vImp_Correct15.at(TABLE_STEP_IMP_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vImp_Correct15.at(j-1) ) * 100 ) /
                         (	sysParm->vImp_Correct15.at(j) - sysParm->vImp_Correct15.at(j-1) );

            rfb = sysParm->vImp_Correct15.at(TABLE_STEP_IMP_CORRECT + j-1) +
                                     ( ( sysParm->vImp_Correct15.at(TABLE_STEP_IMP_CORRECT + j) - sysParm->vImp_Correct15.at(TABLE_STEP_IMP_CORRECT + j-1))* div_factor ) /100;
        }
        rfImpedanceTableMeasured15 = rfb;


        rfa = rfImpedanceTableMeasured15;
        j = fast_upper_bound2(sysParm->vDAC_Correct15, rfa);
        if(j == TABLE_STEP_DAC_CORRECT)
        {
            rfb = sysParm->vDAC_Correct15.at(TABLE_STEP_DAC_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vDAC_Correct15.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vDAC_Correct15.at(j-1));
            rfb = ( sysParm->vDAC_Correct15.at(TABLE_STEP_DAC_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vDAC_Correct15.at(j-1) ) * 100 ) /
                         (	sysParm->vDAC_Correct15.at(j) - sysParm->vDAC_Correct15.at(j-1) );

            rfb = sysParm->vDAC_Correct15.at(TABLE_STEP_DAC_CORRECT + j-1) +
                                     ( ( sysParm->vDAC_Correct15.at(TABLE_STEP_DAC_CORRECT + j) - sysParm->vDAC_Correct15.at(TABLE_STEP_DAC_CORRECT + j-1))* div_factor ) /100;
        }
        rfDACTableMeasured15 = rfb;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////25w//////////////////////////////////////////////////
    else if(mRfDeviceFactors->watt_select == WATT_25)
    {
        rfa = mRfDeviceFactors->rfImpedanceMeasured;
        j = fast_upper_bound2(sysParm->vImp_Correct25, rfa);


        if(j == TABLE_STEP_IMP_CORRECT)
        {
            rfb = sysParm->vImp_Correct25.at(TABLE_STEP_IMP_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vImp_Correct25.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vImp_Correct25.at(j-1));
            rfb = ( sysParm->vImp_Correct25.at(TABLE_STEP_IMP_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vImp_Correct25.at(j-1) ) * 100 ) /
                         (	sysParm->vImp_Correct25.at(j) - sysParm->vImp_Correct25.at(j-1) );

            rfb = sysParm->vImp_Correct25.at(TABLE_STEP_IMP_CORRECT + j-1) +
                                     ( ( sysParm->vImp_Correct25.at(TABLE_STEP_IMP_CORRECT + j) - sysParm->vImp_Correct25.at(TABLE_STEP_IMP_CORRECT + j-1))* div_factor ) /100;
        }
        rfImpedanceTableMeasured25 = rfb;


        rfa = rfImpedanceTableMeasured25;
        j = fast_upper_bound2(sysParm->vDAC_Correct25, rfa);
        if(j == TABLE_STEP_DAC_CORRECT)
        {
            rfb = sysParm->vDAC_Correct25.at(TABLE_STEP_DAC_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vDAC_Correct25.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vDAC_Correct25.at(j-1));
            rfb = ( sysParm->vDAC_Correct25.at(TABLE_STEP_DAC_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vDAC_Correct25.at(j-1) ) * 100 ) /
                         (	sysParm->vDAC_Correct25.at(j) - sysParm->vDAC_Correct25.at(j-1) );

            rfb = sysParm->vDAC_Correct25.at(TABLE_STEP_DAC_CORRECT + j-1) +
                                     ( ( sysParm->vDAC_Correct25.at(TABLE_STEP_DAC_CORRECT + j) - sysParm->vDAC_Correct25.at(TABLE_STEP_DAC_CORRECT + j-1))* div_factor ) /100;
        }
        rfDACTableMeasured25 = rfb;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////50w//////////////////////////////////////////////////
    else if(mRfDeviceFactors->watt_select == WATT_50)
    {
        rfa = mRfDeviceFactors->rfImpedanceMeasured;
        j = fast_upper_bound2(sysParm->vImp_Correct50, rfa);

        if(j == TABLE_STEP_IMP_CORRECT)
        {
            rfb = sysParm->vImp_Correct50.at(TABLE_STEP_IMP_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vImp_Correct50.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vImp_Correct50.at(j-1));
            rfb = ( sysParm->vImp_Correct50.at(TABLE_STEP_IMP_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vImp_Correct50.at(j-1) ) * 100 ) /
                         (	sysParm->vImp_Correct50.at(j) - sysParm->vImp_Correct50.at(j-1) );

            rfb = sysParm->vImp_Correct50.at(TABLE_STEP_IMP_CORRECT + j-1) +
                                     ( ( sysParm->vImp_Correct50.at(TABLE_STEP_IMP_CORRECT + j) - sysParm->vImp_Correct50.at(TABLE_STEP_IMP_CORRECT + j-1))* div_factor ) /100;
        }
        rfImpedanceTableMeasured50 = rfb;


        rfa = rfImpedanceTableMeasured50;
        j = fast_upper_bound2(sysParm->vDAC_Correct50, rfa);
        if(j == TABLE_STEP_DAC_CORRECT)
        {
            rfb = sysParm->vDAC_Correct50.at(TABLE_STEP_DAC_CORRECT + j - 1);
        }
        else if(rfa <  sysParm->vDAC_Correct50.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
        {
            div_factor = (  rfa * 100 ) / (sysParm->vDAC_Correct50.at(j-1));
            rfb = ( sysParm->vDAC_Correct50.at(TABLE_STEP_DAC_CORRECT + j - 1) * div_factor ) /100;
        }
        else
        {
            div_factor = ( ( rfa - sysParm->vDAC_Correct50.at(j-1) ) * 100 ) /
                         (	sysParm->vDAC_Correct50.at(j) - sysParm->vDAC_Correct50.at(j-1) );

            rfb = sysParm->vDAC_Correct50.at(TABLE_STEP_DAC_CORRECT + j-1) +
                                     ( ( sysParm->vDAC_Correct50.at(TABLE_STEP_DAC_CORRECT + j) - sysParm->vDAC_Correct50.at(TABLE_STEP_DAC_CORRECT + j-1))* div_factor ) /100;
        }
        rfDACTableMeasured50 = rfb;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    mRfDeviceFactors->rfImpedanceTableMeasured7 = rfImpedanceTableMeasured7;
    mRfDeviceFactors->rfImpedanceTableMeasured15 = rfImpedanceTableMeasured15;
    mRfDeviceFactors->rfImpedanceTableMeasured25 = rfImpedanceTableMeasured25;
    mRfDeviceFactors->rfImpedanceTableMeasured50 = rfImpedanceTableMeasured50;
    mRfDeviceFactors->rfDACTableMeasured7 = rfDACTableMeasured7;
    mRfDeviceFactors->rfDACTableMeasured15 = rfDACTableMeasured15;
    mRfDeviceFactors->rfDACTableMeasured25 = rfDACTableMeasured25;
    mRfDeviceFactors->rfDACTableMeasured50 = rfDACTableMeasured50;
}

uint16 MonitoringTask::IOADCSPIPort(int32 n_cs, int32 channel)
{
    uint16 adc_value = 0;
    Q_UNUSED(n_cs);
    switch(channel)
    {
        case 2: //voltage
            //adc_value = mRfDeviceFactors->tmReadADCVoltage++;
            if(adc_voltage++ > 60000)
            {
                adc_voltage = 0;
            }
            adc_value = adc_voltage;

            break;

        case 1: //current
            //adc_value = mRfDeviceFactors->tmReadADCCurrent++;
            if(adc_current++ > 40000)
            {
                adc_current = 0;
            }
            adc_value = adc_current;
            break;

        case 0: //temperature
            //adc_value = mRfDeviceFactors->tmReadADCTemperature++;
            if(adc_temp++> 2000)
            {
                adc_temp = 0;
            }
            adc_value = adc_temp;
            break;

    }

#if 0
    //if(channel == 0)
    {
        mRfDeviceFactors->tmReadADCTemperature++;
        if(sysParm->mRfDeviceFactors->tmReadADCTemperature > 60000)
            sysParm->mRfDeviceFactors->tmReadADCTemperature = 0;
        //else if(sysParm->mRfDeviceFactors->tmReadADCVoltage >  55279)
//           sysParm->mRfDeviceFactors->tmReadADCVoltage = 10000;
    }
    return mRfDeviceFactors->tmReadADCTemperature;
#endif
    return adc_value;
}



uint16 MonitoringTask::MedianFilter(int32 deviceid, int32 index, uint16 data)
{
    uint16 i,j,tmp;
    uint16 *p_sort, *p_data;


    switch(index)
    {
        case 0:
            p_data = static_cast<uint16*>(tm_median_voltage[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_voltage_sort[deviceid]);
            break;

        case 1:
            p_data = static_cast<uint16*>(tm_median_current[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_current_sort[deviceid]);
            break;

        case 2:
            p_data = static_cast<uint16*>(tm_median_tempA[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_temp_sortA[deviceid]);
            break;

        case 3:
            p_data = static_cast<uint16*>(tm_median_tempB[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_temp_sortB[deviceid]);
            break;

    }

    //Filtering
    p_sort[0] = p_data[0]=   p_data[1];
    p_sort[1] = p_data[1]=   p_data[2];
    p_sort[2] = p_data[2]=   data;
    //sort
    for( i =0; i < MEDIAN_ORDER - 1; i++)
    {
        for( j =i + 1; j < MEDIAN_ORDER; j++)
        {
            if( p_sort[i] < p_sort[j] )
            {
                tmp = p_sort[j];
                p_sort[j] = p_sort[i];
                p_sort[i] =tmp ;
            }
        }
    }

    //Get Median
    return p_sort[1];

}



void MonitoringTask::slot_dispCounter()
{
    qDebug("MonitoringTask run %d per 1seconds", counter);
    counter = 0;
}


void MonitoringTask::run()
{

    /*
     *  Start call function.
     *  Synchronous call function 일반적인 처리는 동기로
     *  Sensing adc
     */

    GetAmplifierParameters();

    /*
     *  End call function.
     */


#if __INTERVAL_TEST__
    counter++;
#endif

}
