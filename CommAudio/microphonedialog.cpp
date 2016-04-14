#include "microphonedialog.h"
#include "ui_microphonedialog.h"
#include "client.h"


MicrophoneDialog::MicrophoneDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MicrophoneDialog)
{
    ui->setupUi(this);
    //MicrophoneDialog *md = this;
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

    //destinationFile.setFileName("test.raw");
    //destinationFile.open( QIODevice::WriteOnly | QIODevice::Truncate );

    info = QAudioDeviceInfo::defaultInputDevice();

    devicesAvailable = info.availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo device, devicesAvailable) {
        QString devName = device.deviceName();
        ui->inputBox->addItem(devName, QVariant(devName));
    }

    QBuffer *iBuffer;
//    buffer = (char*)malloc(MAX_BUF);

    QAudioFormat format;
    format.setSampleRate(8000);
    format.setCodec("audio/pcm");
    format.setSampleSize(16);

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


    if (indexSelected == -1) {
        audioInput = new QAudioInput(format,this);
        audioOutput = new QAudioOutput(QAudioDeviceInfo::defaultInputDevice(),format, this);
    }
    else {
        audioInput = new QAudioInput(devSelected,format,this);
        audioOutput = new QAudioOutput(QAudioDeviceInfo::defaultInputDevice(), format, this);
    }

    isRecording = true;
    iBuffer = new QBuffer();
    iBuffer->open(QIODevice::ReadWrite);
    audioInputDevice = audioInput->start();
    startMicrophone(ui->ipAddress_textedit->toPlainText().toStdString().c_str(), this, &(((MainWindow*)parent())->_bm));


}


void MicrophoneDialog::on_pushButton_clicked()
{
    isRecording = false;
    audioInput->stop();
    audioOutput->stop();
    //destinationFile.close();
}
