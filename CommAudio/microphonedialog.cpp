#include "microphonedialog.h"
#include "ui_microphonedialog.h"
#include "client.h"
#include <QAudioInput>
#include <QIODevice>
#include <QFile>

QAudioInput *audio;
QFile destinationFile;
QAudioDeviceInfo info;
QList<QAudioDeviceInfo> devicesAvailable;
char * buffer;
MicrophoneDialog::MicrophoneDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MicrophoneDialog)
{
    ui->setupUi(this);

    info = QAudioDeviceInfo::defaultInputDevice();

    devicesAvailable = info.availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo device, devicesAvailable) {
        QString devName = device.deviceName();
        ui->inputBox->addItem(devName, QVariant(devName));
    }

//    audioRecorder = new QAudioRecorder(this);
//    probe = new QAudioProbe;

//    probe->setSource(audioRecorder);

//
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

//    destinationFile.setFileName("test.raw");
//    destinationFile.open( QIODevice::WriteOnly | QIODevice::Truncate );

    QBuffer *iBuffer;
//    buffer = (char*)malloc(MAX_BUF);

    QAudioFormat format;
    format.setSampleRate(8000);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    int indexSelected = ui->inputBox->currentIndex();

    QAudioDeviceInfo devSelected = devicesAvailable.takeAt(indexSelected);
    qDebug() << devSelected.deviceName();


    if (!info.isFormatSupported(format)){
        format = info.nearestFormat(format);
    }

    qDebug()<<format.sampleRate();
    qDebug()<<format.codec();
    qDebug()<<format.byteOrder();
    qDebug()<<format.sampleType();


    if (indexSelected == -1)
        audio = new QAudioInput(format,this);
    else
        audio = new QAudioInput(devSelected,format,this);


    iBuffer = new QBuffer();
    iBuffer->open(QIODevice::WriteOnly);
    audio->start(iBuffer);
    startMicrophone(ui->ipAddress_textedit->toPlainText().toStdString().c_str(),iBuffer->buffer().data());


}


void MicrophoneDialog::on_pushButton_clicked()
{
    audio->stop();
    //destinationFile.close();
}
