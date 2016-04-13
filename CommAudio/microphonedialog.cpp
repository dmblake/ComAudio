#include "microphonedialog.h"
#include "ui_microphonedialog.h"
#include "client.h"
#include <QAudioInput>
#include <QIODevice>
#include <QFile>

QAudioInput *audio;
QFile destinationFile;
char * buffer;
MicrophoneDialog::MicrophoneDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MicrophoneDialog)
{
    ui->setupUi(this);

//    audioRecorder = new QAudioRecorder(this);
//    probe = new QAudioProbe;

//    probe->setSource(audioRecorder);

//    ui->inputBox->addItem(tr("Default"), QVariant(QString()));
//    foreach (const QString &device, audioRecorder->audioInputs()) {
//        ui->inputBox->addItem(device, QVariant(device));
//    }
}

MicrophoneDialog::~MicrophoneDialog()
{
    delete ui;
}

void MicrophoneDialog::on_startButton_clicked()
{
    startMicrophone(ui->ipAddress_textedit->toPlainText().toStdString().c_str());
//    destinationFile.setFileName("test.raw");
//    destinationFile.open( QIODevice::WriteOnly | QIODevice::Truncate );

    QBuffer *iBuffer;
//    buffer = (char*)malloc(MAX_BUF);

    QAudioFormat format;
    format.setSampleRate(8000);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format))
        format = info.nearestFormat(format);

    audio = new QAudioInput(format,this);


    iBuffer = new QBuffer();
    iBuffer->open(QIODevice::WriteOnly);
    audio->start(iBuffer);

}


void MicrophoneDialog::on_pushButton_clicked()
{
    audio->stop();
    destinationFile.close();
}
