#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"
#include "FileUtil.h"
#include "client.h"
#include <QFile>
#include <QTextStream>

bool isServer;
// hank revision
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    ui->setupUi(this);


    mw = this;
}


MainWindow::MainWindow(bool server,bool client, QString ipaddr):_server(server),_client(client),_ipaddr(ipaddr),
    ui(new Ui::MainWindow){

    QFile f(":qdarkstyle/style.qss");
    if (!f.exists())
    {
        printf("Unable to set stylesheet, file not found\n");
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }


    ui->setupUi(this);
    mw = this;


    ui->menuBar->setCornerWidget(ui->close,Qt::TopRightCorner);
    this->setWindowFlags(Qt::FramelessWindowHint);

    if(_server == true){
        ui->tabWidget->setTabEnabled(1,false);
        startServer();
        isServer = true;
    }
    else if (_client == true){
        ui->tabWidget->setTabEnabled(0,false);
        setupTcpSocket(ipaddr);
        isServer = false;
    }

    qDebug() << server;
    qDebug() << client;
    qDebug() << ipaddr;
}

MainWindow::~MainWindow()
{
    delete ui;
}


//hank
void MainWindow::printToListView(std::string msg)
{
    if(_server == true)
        ui->listWidget_debug_server->addItem(QString::fromStdString(msg));
    if(_client == true)
        ui->listWidget_debug_client->addItem(QString::fromStdString(msg));
}


void MainWindow::on_updateButton_clicked()
{
    startFileTransfer();
}

void MainWindow::on_close_clicked()
{
    qApp->exit();
}


void MainWindow::on_playButton_server_clicked()
{
    startServerMulticastSession();
}

void MainWindow::on_playButton_client_clicked()
{
    startClientMulticastSession();
}


void MainWindow::on_refreshButton_clicked()
{
    std::string extension = ".mp3";
    std::string serverList = listAllFiles(extension);
    QString str = QString::fromStdString(serverList);
    ui->listWidget->addItem(str);
    qDebug()<< str;
}


//void MainWindow::on_microphoneButton_server_clicked()
//{

//    //MircophoneDialog c(this);
//}
