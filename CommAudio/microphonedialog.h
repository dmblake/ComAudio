#ifndef MICROPHONEDIALOG_H
#define MICROPHONEDIALOG_H

#include <QDialog>
#include <QBuffer>
#include <QAudioOutput>
#include <QAudioInput>
#include <QIODevice>
#include <QFile>
#include "shared.h"

namespace Ui {
class MicrophoneDialog;
}

class MicrophoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MicrophoneDialog(QWidget *parent = 0);
    QIODevice *audioInputDevice;
    QIODevice *audioOutputDevice;
    QAudioInput *audioInput;
    QAudioOutput *audioOutput;
    bool isRecording = false;
    ~MicrophoneDialog();

private slots:
    void on_startButton_clicked();

    void on_stopButton_clicked();

private:
    Ui::MicrophoneDialog *ui;
    QAudioFormat format;
    QFile destinationFile;
    QAudioDeviceInfo info;
    QList<QAudioDeviceInfo> devicesAvailable;
    char * buffer;
};

#endif // MICROPHONEDIALOG_H
