#include "microphonedialog.h"
#include "ui_microphonedialog.h"
#include "client.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:         mainwindow.cpp - Part of an application that will capture, send and receiveThread
--                                       microphone data.
--
-- PROGRAM:             CommAudio
--
-- FUNCTIONS:           explicit MainWindow(QWidget *parent = 0);
--                      QIODevice *audioInputDevice;
--                      QIODevice *audioOutputDevice;
--                      QAudioInput *audioInput;
--                      QAudioOutput *audioOutput;
--                      bool isRecording = false;
--                      ~MainWindow();
--                      void on_startButton_clicked();
--                      void on_pushButton_clicked();
--
--
-- DATE:                April 11th 2016 - Start functionality for microphone
--
-- REVISIONS:           April 12th 2016 - Finished functionality for microphone capturing
--                      April 13th 2016 - Finished sending microphone data over network
--
-- DESIGNER:            Dhivya Manohar
--
-- PROGRAMMER:          Dhivya Manohar, Dylan Blake, Joseph Tam
--
-- NOTES:               This part of the program should be able to capture data through a microphone
--                      and send it over the network while simutaneously hear the incoming data from other clients.
--                      When this dialog shows up, you have to type in a IP address of the person you want to have
--                      a conversation with. There is a start button to start listening to the client of your choice
--                      and start talking, and stop button to stop listening and transfer data.
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:        MainWindow
--
-- DATE:            April 11th 2016
--
-- REVISIONS:       N/A
--
-- DESIGNER:        Dhivya Manohar
--
-- PROGRAMMER:      Dhivya Manohar
--
-- INTERFACE:       MicrophoneDialog::MicrophoneDialog(QWidget *parent) :
--                  QDialog(parent), ui(new Ui::MicrophoneDialog)
--
-- RETURNS:         void
--
-- NOTES:           This is a constructor for the MicrophoneDialog class. Call this function to create a new
--                  microphone dialog that pops up when you want to start a voice conversation.This function also
--                  populates the list of available device that you can use choose from to capture your beautiful
--                  voice.
----------------------------------------------------------------------------------------------------------------------*/
MicrophoneDialog::MicrophoneDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MicrophoneDialog)
{
    ui->setupUi(this);

    info = QAudioDeviceInfo::defaultInputDevice(); //setting input device

    devicesAvailable = info.availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo device, devicesAvailable) {
        QString devName = device.deviceName();
        ui->inputBox->addItem(devName, QVariant(devName));
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:        ~MicrophoneDialog
--
-- DATE:            April 11th 2016
--
-- REVISIONS:       N/A
--
-- DESIGNER:        Dhivya Manohar
--
-- PROGRAMMER:      Dhivya Manohar
--
-- INTERFACE:       ~MicrophoneDialog()
--
-- RETURNS:         void
--
-- NOTES:           This is a deconstructor for the MicrophoneDialog.
----------------------------------------------------------------------------------------------------------------------*/
MicrophoneDialog::~MicrophoneDialog()
{
    delete ui;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:        on_startButton_clicked
--
-- DATE:            April 11th 2016
--
-- REVISIONS:       April 12th 2016 - Got local capturing of data from a microphone to computer
--                  April 13th 2016 - Sends captured data to network
--
-- DESIGNER:        Dhivya Manohar
--
-- PROGRAMMER:      Dhivya Manohar, Dylan Blake
--
-- INTERFACE:       on_startButton_clicked()
--
-- RETURNS:         void
--
-- NOTES:           This function allows functionality for a button that allows data to follow from one client
--                  to another.
----------------------------------------------------------------------------------------------------------------------*/
void MicrophoneDialog::on_startButton_clicked()
{
    info = QAudioDeviceInfo::defaultInputDevice();

    //displays all available devices
    devicesAvailable = info.availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo device, devicesAvailable) {
        QString devName = device.deviceName();
        ui->inputBox->addItem(devName, QVariant(devName));
    }


    QAudioFormat format; //sets format you want your audio to be captured at
    format.setSampleRate(8000);
    format.setCodec("audio/pcm");
    format.setSampleSize(16);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    int indexSelected = ui->inputBox->currentIndex();


    QAudioDeviceInfo devSelected = devicesAvailable.takeAt(indexSelected);


    if (!info.isFormatSupported(format)){
        format = info.nearestFormat(format);
    }


    if (indexSelected == -1) {
        audioInput = new QAudioInput(format,this);
        audioOutput = new QAudioOutput(QAudioDeviceInfo::defaultInputDevice(),format, this);
    }
    else {
        audioInput = new QAudioInput(devSelected,format,this);
        audioOutput = new QAudioOutput(QAudioDeviceInfo::defaultInputDevice(), format, this);
    }

    isRecording = true; //setting flag
    audioInputDevice = audioInput->start(); //starts capturing data
    startMicrophone(ui->ipAddress_textedit->toPlainText().toStdString().c_str(), this, &(((MainWindow*)parent())->_bm)); //sends captured data


}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:        on_stophButton_clicked
--
-- DATE:            April 11th 2016
--
-- REVISIONS:       April 12th 2016 - Stops local capturing of data from a microphone to computer
--                  April 13th 2016 - Stops sending data to network
--
-- DESIGNER:        Dhivya Manohar
--
-- PROGRAMMER:      Dhivya Manohar, Dylan Blake
--
-- INTERFACE:       on_stopButton_clicked()
--
-- RETURNS:         void
--
-- NOTES:           This function allows functionality for a button that allows data to stop sending to other clients.
----------------------------------------------------------------------------------------------------------------------*/
void MicrophoneDialog::on_stopButton_clicked()
{
    isRecording = false; //sets flag to false
    audioInput->stop();
    audioOutput->stop();
}
