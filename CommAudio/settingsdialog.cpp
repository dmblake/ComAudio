#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "mainwindow.h"

settingsDialog::settingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsDialog)
{

    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    ui->clientText->setText("192.168.0.23");

}

settingsDialog::~settingsDialog()
{
    delete ui;
}

void settingsDialog::on_continueButton_clicked(){

    MainWindow* w = new MainWindow(ui->serverCheck->isChecked(), ui->clientCheck->isChecked(), ui->clientText->toPlainText());
    w->show();

    close();
}
