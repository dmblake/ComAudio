#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "mainwindow.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:         settingsdialog.cpp - Beginning of the program when it prompts you of the type of user
--                                           your are.
--
-- PROGRAM:             CommAudio
--
-- FUNCTIONS:               explicit settingsDialog(QWidget *parent = 0);
--                          ~settingsDialog();
--                          void on_continueButton_clicked();
--
--
-- DATE:                March 21st 2016 - Start/finished functionality for settings dialog
--
-- REVISIONS:           March 29th 2016 - Fixed crashing bug
--
-- DESIGNER:            Dhivya Manohar
--
-- PROGRAMMER:          Dhivya Manohar
--
-- NOTES:               This is the beginning of the program that prompts user to select for the type of user
--                      they are.
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:        settingsDialog
--
-- DATE:            March 21st 2016
--
-- REVISIONS:       N/A
--
-- DESIGNER:        Dhivya Manohar
--
-- PROGRAMMER:      Dhivya Manohar
--
-- INTERFACE:       settingsDialog::MicrophoneDialog(QWidget *parent) :
--                  QDialog(parent), ui(new Ui::settingsDialog)
--
-- RETURNS:         void
--
-- NOTES:           This is a constructor for the settingsDialog class.
----------------------------------------------------------------------------------------------------------------------*/
settingsDialog::settingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsDialog)
{

    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    ui->clientText->setText("192.168.0.23");

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:        ~settingsDialog
--
-- DATE:            March 21st 2016
--
-- REVISIONS:       N/A
--
-- DESIGNER:        Dhivya Manohar
--
-- PROGRAMMER:      Dhivya Manohar
--
-- INTERFACE:       ~settingsDialog()
--
-- RETURNS:         void
--
-- NOTES:           This is a deconstructor for the settingsDialog class.
----------------------------------------------------------------------------------------------------------------------*/
settingsDialog::~settingsDialog()
{
    delete ui;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:        on_continueButton_clicked
--
-- DATE:            March 21st 2016
--
-- REVISIONS:       March 29th 2016 - fixed crashing bug
--
-- DESIGNER:        Dhivya Manohar
--
-- PROGRAMMER:      Dhivya Manohar
--
-- INTERFACE:       on_continueButton_clicked()
--
-- RETURNS:         void
--
-- NOTES:           This function allows functionality for a button to create the main window of the program if
--                  they are able to connect successfully. If not able to connect, the window will stay still until
--                  it finds an available connection.
----------------------------------------------------------------------------------------------------------------------*/
void settingsDialog::on_continueButton_clicked(){

    MainWindow* w = new MainWindow(ui->serverCheck->isChecked(), ui->clientCheck->isChecked(), ui->clientText->toPlainText());
    w->show();

    close();
}
