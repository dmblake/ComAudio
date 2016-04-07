#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "mainwindow.h"

settingsDialog::settingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsDialog)
{

    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

}

settingsDialog::~settingsDialog()
{
    delete ui;
}

void settingsDialog::on_continueButton_clicked(){

    MainWindow *w = new MainWindow(this);
    w->show();

    close();
}
