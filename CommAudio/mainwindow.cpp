#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"
#include "FileUtil.h"
#include "client.h"
#include "microphonedialog.h"
#include <QFile>
#include <QTextStream>


bool isServer;

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:         MainWindow.cpp - The main window of the
--
-- PROGRAM:             CommAudio
--
-- FUNCTIONS:           explicit settingsDialog(QWidget *parent = 0);
--                      ~settingsDialog();
--                      void on_continueButton_clicked();
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
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _bm(MAX_BUF, false)
{


    ui->setupUi(this);


    mw = this;
}


MainWindow::MainWindow(bool server,bool client, QString ipaddr):_server(server),_client(client),_ipaddr(ipaddr),
    ui(new Ui::MainWindow),
    _bm(MAX_BUF, server)
{

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
        ui->listWidget->clear();
        std::string serverList = listAllFiles(".wav");
        serverList += listAllFiles(".mp3");
        serverList += listAllFiles(".raw");
        std::vector<std::string> list = split(serverList, '\n');
        for (std::string elem : list) {
            QString str = QString::fromStdString(elem);
            ui->playlistWidget->addItem(str);
        }

    }

    // refresh file list
    on_refreshButton_clicked();

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
   std::vector<std::string> filesReceived = updateServerFiles();
   ui->listWidget_serverFiles->clear();
   for(auto elem : filesReceived) {
       ui->listWidget_serverFiles->addItem(QString::fromStdString(elem));
    }
}

void MainWindow::on_close_clicked()
{
    qApp->exit();
}


void MainWindow::on_playButton_server_clicked()
{
    start_playing();
}

void MainWindow::start_playing() {
    if (!_bm._isPlaying) {
        _bm._isPlaying = true;
        _bm.startReadThread((LPVOID)&_bm);
        _bm.startPlayThread((LPVOID)NULL);
        // start server
        if (_bm._isServer) {
            _bm._isSending = true;
            startServerMulticastSession(&_bm);
        }
    } else {
        // check for pause and resume
        _bm.resume();
    }
}

void MainWindow::on_playButton_client_clicked()
{

    // LOGIC FOR LOCAL PLAYBACK VS RADIO HERE
    startClientMulticastSession(&_bm);
    start_playing();
}

// hank revis
void MainWindow::on_refreshButton_clicked()
{
    ui->listWidget->clear();
    std::string serverList = listAllFiles(".wav");
    serverList += listAllFiles(".mp3");
    serverList += listAllFiles(".raw");
    std::vector<std::string> list = split(serverList, '\n');
    for (std::string elem : list) {
        QString str = QString::fromStdString(elem);
        ui->listWidget->addItem(str);
    }
}


void MainWindow::on_microphoneButton_server_clicked()
{

    MicrophoneDialog c(this);
}

// client side item selection
void MainWindow::on_listWidget_2_itemClicked(QListWidgetItem *item)
{
    // reuse functionality from server side
    on_listWidget_itemClicked(item);
}

// server side item selection
void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    std::string txt = item->text().toUtf8().constData();
    std::vector<std::string> vec = split(txt, ',');
    _bm.setFilename(vec[0].c_str());
}

void MainWindow::on_downloadButton_clicked()
{

    QListWidgetItem *selected = ui->listWidget_serverFiles->currentItem();
    std::string songToDownload = selected->text().toStdString();
    downloadFile(songToDownload.c_str());

    ui->playlistWidget->clear();
    std::string serverList = listAllFiles(".wav");
    serverList += listAllFiles(".mp3");
    std::vector<std::string> list = split(serverList, '\n');
    for (std::string elem : list) {
        QString str = QString::fromStdString(elem);
        ui->playlistWidget->addItem(str);
    }
}

void MainWindow::on_stopButton_server_clicked()
{
    _bm.stop();
}

void MainWindow::on_pauseButton_server_clicked()
{
    _bm.pause();
}

void MainWindow::on_stopButton_client_clicked()
{
    _bm.stop();
}

void MainWindow::on_muteButtonServer_clicked()
{
    _bm.mute(); 
}


void MainWindow::on_microphoneButton_client_clicked()
{
    MicrophoneDialog c(this);
    c.setModal(true);
    c.exec();
}

void MainWindow::on_playlistWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if (!_bm._isPlaying) {
        _bm._isPlaying = true;
        _bm.startReadThread((LPVOID)&_bm);
        _bm.startPlayThread((LPVOID)NULL);
     } else {
        // check for pause and resume
        _bm.resume();
    }
}

void MainWindow::on_playlistWidget_itemClicked(QListWidgetItem *item)
{
    std::string txt = item->text().toUtf8().constData();
    std::vector<std::string> vec = split(txt, ',');
    _bm.setFilename(vec[0].c_str());
}
