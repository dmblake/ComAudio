#include "microphonedialog.h"
#include "ui_microphonedialog.h"


MicrophoneDialog::MicrophoneDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MicrophoneDialog)
{
    ui->setupUi(this);

    audioRecorder = new QAudioRecorder(this);
    probe = new QAudioProbe;

    probe->setSource(audioRecorder);

    ui->inputBox->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &device, audioRecorder->audioInputs()) {
        ui->inputBox->addItem(device, QVariant(device));
    }
}

MicrophoneDialog::~MicrophoneDialog()
{
    delete ui;
}
