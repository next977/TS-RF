#include "audiooutput.h"
#include <QCoreApplication>
#include <stdio.h>
#include <QAudioFormat>
#include <QDebug>
#include <QtEndian>
#include <QBuffer>
#include <QAudioOutput>
#include <QFile>
AudioOutput::AudioOutput(QObject *parent) : QObject(parent)
{
    /*
    QAudioDeviceInfo d1;
    QList<QAudioDeviceInfo> l1 = d1.availableDevices(QAudio::AudioOutput);

    qDebug() << "======================================================";
    qDebug() << l1.first().supportedCodecs();
    qDebug() << l1.first().supportedChannelCounts();
    qDebug() << l1.first().supportedSampleTypes();
    qDebug() << l1.first().supportedSampleRates();
    qDebug() << l1.first().supportedSampleSizes();
    */
    QAudioFormat desiredFormat;
    desiredFormat.setChannelCount(1);
    desiredFormat.setByteOrder(QAudioFormat::LittleEndian);
    desiredFormat.setCodec("audio/pcm");
    desiredFormat.setSampleType(QAudioFormat::SignedInt);
#ifndef _ONPC_
    desiredFormat.setSampleRate(44100000);
#else
    desiredFormat.setSampleRate(96000);
#endif

    desiredFormat.setSampleSize(16);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(desiredFormat))
    {
           qWarning() << "Default format not supported, trying to use the nearest.";
           desiredFormat = info.preferredFormat();
    }

    audio = new QAudioOutput(desiredFormat);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));


}

AudioOutput::~AudioOutput()
{
    if(audio)delete audio;
}
/*
void AudioOutput::slotProbeBuffer(const QAudioBuffer &buffer)
{
    QByteArray byteArray;
    byteArray.append((char *)buffer.data());
    QBuffer mbuffer(&byteArray);

    mbuffer.open(QIODevice::WriteOnly);
    audio->start(&mbuffer);

    mbuffer.close();
}
*/

void AudioOutput::handleStateChanged(QAudio::State _state)
{
    qDebug() << "handleStateChanged : " << _state;

    switch (_state) {
        case QAudio::ActiveState:
            // Finished playing (no more data)
            qDebug("Active State");
            break;
        case QAudio::IdleState:
            // Finished playing (no more data)
            qDebug("Idle State");
            /*
            if((SND_state == SND_START)||(SND_state == SND_RFSTART))
            {
                playSound(SND_RFSTART);
            }
            */
            if(SND_state == SND_RFSTART || SND_state == SND_RFOUTPUT)
            {

                audio->stop();
                sourceFile.close();
                playSound(SND_RFOUTPUT);
            }
            break;
        
        case QAudio::StoppedState:
            // Stopped for other reasons
            qDebug("Stopped State");
            if (audio->error() != QAudio::NoError) {
                // Error handling
                 qDebug("QAudio Error");
            }
            audio->stop();
            sourceFile.close();

            break;

        default:
            // ... other cases as appropriate
            break;
    }
}

void AudioOutput::playSound(AudioOutput::eAUDIOS state)
{
    uint32 singletimer_interval;
    SND_state = state;
    switch(state)
    {
    ////////////////////// qDebug 를 하게 되면 underrun error 발생 , 다른 code 넣지 말기! ///////////////////////////
        case SND_RFSTART:
            sourceFile.setFileName(":/sound/rf_start.wav");
            singletimer_interval = 500; // 0.5s
            break;
        case SND_RFSTOP:
            sourceFile.setFileName(":/sound/rf_stop.wav");
            singletimer_interval = 500; // 0.5s
            break;
        case SND_RFOUTPUT:
            sourceFile.setFileName(":/sound/rf_output.wav");
            singletimer_interval = 500; // 0.5s
            break;
        case SND_BOOTING:
            file_name = ":/sound/booting.wav";
            sourceFile.setFileName(":/sound/booting.wav");
            singletimer_interval = 1000; //1s
            break;
        case SND_CLICK_BUTTON:
            file_name = ":/sound/click_button.wav";
            sourceFile.setFileName(":/sound/click_button.wav");
            singletimer_interval = 500; // 0.5s
            break;
        case SND_UP_BUTTON:
            sourceFile.setFileName(":/sound/up_button.wav");
            //singletimer_interval = 100; // 0.5s
            break;
        case SND_DOWN_BUTTON:
            sourceFile.setFileName(":/sound/down_button.wav");
            //singletimer_interval = 100; // 0.5s
            break;
        case SND_ALARM:
            sourceFile.setFileName(":/sound/alarm.wav");
            singletimer_interval = 2000; //1s
            break;
        case SND_ERROR:
            sourceFile.setFileName(":/sound/error.wav");
            singletimer_interval = 2000; //1s
            break;


        case SND_STOP:
            audio->stop();
            sourceFile.close();

#ifndef _ONPC_
            //gpioTools::AudioAMPControl_Disable();
#endif

            // qDebug() << "SND_RFSTOP" ;
            return ;

            break;

        default :
            break;
    }
#if 0
    gpioTools::AudioAMPControl_Enable();
#endif
#if 1
    sourceFile.open(QIODevice::ReadOnly);
    audio->start(&sourceFile);
    if(state == SND_UP_BUTTON || state ==SND_DOWN_BUTTON )
    {

    }
    else
    {
        QTimer::singleShot(singletimer_interval, this, SLOT(slot_stopSound()));
    }
#endif

    //file_name = ":/sound/booting.wav";
    //audio_test();
#if 0
    qDebug("file_name = %s", file_name);
    QFile audio_file(file_name);
    QByteArray audio_Buffer;

    if(audio_file.open(QIODevice::ReadOnly))
    {
        char stream[4] = {0,};
        audio_file.read(stream, 4); //  Same with "RIFF"   not use
        audio_file.read(stream, 4); //  Chumk size         not use
        audio_file.read(stream, 4); //  Format             not use
        audio_file.read(stream, 4); //  Sub chunk1 ID      not use
        audio_file.read(stream, 4); //  Sub chunk1 size    not use
        audio_file.read(stream, 2); //  Audio Format       not use
        memset(stream,0, 4);
        audio_file.read(stream, 2); //  Channel number    use
        //Channels = qFromLittleEndian<quint16>((uchar*)stream);
        memset(stream,0, 4);

        audio_file.read(stream, 4); //  Sample rate       use

        //SampleRate = qFromLittleEndian<quint32>((uchar*)stream);

        audio_file.read(stream, 4); //  Byte rate         not use
        audio_file.read(stream, 2); //  Byte Allign       not use(아마도 엔디언 설정인듯)
        memset(stream,0, 4);
        audio_file.read(stream, 2); //  BPS               use
        //BPS = qFromLittleEndian<quint16>((uchar*)stream);
        audio_file.read(stream, 4); //  Sub chunk2 ID      not use

        audio_file.read(stream, 4); //  Sub chunk2 size    not use


        audio_Buffer.clear();

        while(!audio_file.atEnd())

        {

            char s[1];

            audio_file.read(s,1);

            audio_Buffer.append(s[0]);

        }

        audio_file.close();

        QBuffer audio_buffer(&audio_Buffer);

        audio_buffer.open(QIODevice::ReadOnly);
        audio->start(&audio_buffer);
    }
#endif
#if 1
    if(sourceFile.isOpen()==false)
    {       

        audio->stop();
        sourceFile.close();
        bool p = sourceFile.open(QIODevice::ReadOnly);
#ifndef _ONPC_
        //gpioTools::AudioAMPControl_Enable();
#endif
        audio->setVolume(1.0); //no apply
        audio->start(&sourceFile);
#if 0
       QEventLoop loop;
       QObject::connect(audio, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));

       do {
            loop.exec();
        } while(audio->state() == QAudio::ActiveState);

      // while(audio->state() == QAudio::ActiveState) loop.exec();
       //loop.quit();
#endif

    }
#endif



}
void AudioOutput::slot_stopSound()
{

    audio->stop();
    sourceFile.close();

#if 0
    gpioTools::AudioAMPControl_Disable();
#endif
}

WavFileDevice::WavFileDevice()
{

}

WavFileDevice::~WavFileDevice()
{

}

qint64 WavFileDevice::readData(char *data, qint64 maxlen)
{
    //qDebug() << maxlen;
    return QFile::readData(data, maxlen);
}

