#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QFile>

#include <QAudioOutput>
#include <QTimer>
#include <QAudioBuffer>
#include <QAudioOutput>
#include <QAudioProbe>
#include <QCoreApplication>
#include "gpiotools.h"

class WavFileDevice : public QFile
{
    Q_OBJECT

public:
    WavFileDevice();
    ~WavFileDevice();
    qint64 readData(char *data, qint64 maxlen) override;


};

class AudioOutput : public QObject
{
    Q_OBJECT
public:
    enum eAUDIOS{
        SND_RFOUTPUT = 0,
        SND_RFSTART,
        SND_RFSTOP,
        SND_STOP,
        SND_CLICK_BUTTON,
        SND_BOOTING,
        SND_UP_BUTTON,
        SND_DOWN_BUTTON,
        SND_ALARM,
        SND_ERROR

        };

    explicit AudioOutput(QObject *parent = 0);
    virtual ~AudioOutput();
    void playSound(AudioOutput::eAUDIOS state);
    void stopSound();


private:
   // WavFileDevice sourceFile;
    QFile sourceFile;
    QString file_name;
    QAudioOutput* audio;
    eAUDIOS SND_state;


signals:

public slots:
    void slot_stopSound();
    void handleStateChanged(QAudio::State _state);
  //  void slotProbeBuffer(const QAudioBuffer &buffer);

};

#endif // AUDIOOUTPUT_H
