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
        startClient(this);
        isServer = true;
    }
    else if (_client == true){
        ui->tabWidget->setTabEnabled(0,false);
        setupTcpSocket(ipaddr);
        isServer = false;
        startClient(this);
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
   std::vector<std::string> filesReceived = updateServerFiles();
   ui->listWidget_serverFiles->clear();
   for(auto elem : filesReceived) {
       // each vector will have 2 elements - the file name, and the size
       std::vector<std::string> singleFnameAndSize = split(elem, ',');
       ui->listWidget_serverFiles->addItem(QString::fromStdString(singleFnameAndSize[0]));
       // access singleFnameAndSize[0] to get the filename
       // access singleFnameAndSize[1] to get the size in string form
       // update your listwidget thingy here
    }
}

void MainWindow::on_close_clicked()
{
    qApp->exit();
}


void MainWindow::on_playButton_server_clicked()
{
    start_playing();
    // perform any other server related actions

    /*  PRESERVED FOR REFERENCE : CHANGING STATE OF MAINWINDOW IS DEPRECATED : USE BUFFERMANAGER _BM
    switch (_playingState) {
    case BASS_ACTIVE_PLAYING:
        break;
    case BASS_ACTIVE_STOPPED:
        _playingState = BASS_ACTIVE_PLAYING;
        playback();
        break;
    case BASS_ACTIVE_PAUSED:
        _playingState = BASS_ACTIVE_PLAYING;
        break;
    case -1: // first time only
        startServerMulticastSession();
        _playingState = BASS_ACTIVE_PLAYING;
        playback();
    }
    */
}

void MainWindow::start_playing() {
    if (!_bm._isPlaying) {
        _bm._isPlaying = true;
        _bm.startReadThread((LPVOID)&_bm);
        _bm.startPlayThread((LPVOID)NULL);
        // start server
        if (_bm._isServer) {
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
    startClientMulticastSession();
    start_playing();
}

// hank revis
void MainWindow::on_refreshButton_clicked()
{
    ui->listWidget->clear();
    std::string serverList = listAllFiles(".wav");
    serverList += listAllFiles(".mp3");
    std::vector<std::string> list = split(serverList, '\n');
    for (std::string elem : list) {
        QString str = QString::fromStdString(elem);
        ui->listWidget->addItem(str);
    }

    qDebug()<< serverList.c_str();
}


//void MainWindow::on_microphoneButton_server_clicked()
//{

//    //MircophoneDialog c(this);
//}

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
    //qDebug() << songToDownload.c_str();
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
    changePlayback(BASS_ACTIVE_STOPPED);
    _bm.stop();
}

// allow playback
bool MainWindow::isPlaying() {
    return _playing;
}

// change playback status
void MainWindow::setPlaying(bool val) {
    _playing = val;
}

void MainWindow::on_pauseButton_server_clicked()
{
    if (_playingState == BASS_ACTIVE_PLAYING)
        changePlayback(BASS_ACTIVE_PAUSED);
    _bm.pause();

}
