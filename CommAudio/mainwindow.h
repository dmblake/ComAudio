#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "buffermanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    explicit MainWindow(bool server,bool client, QString ipaddr);
    ~MainWindow();
    void printToListView(std::string msg);

    bool isPlaying();
    void setPlaying(bool val);
    int _playingState = -1;

private slots:

    void on_updateButton_clicked();
    void on_close_clicked();

    void on_playButton_server_clicked();

    void on_playButton_client_clicked();
    void on_refreshButton_clicked();

    void on_listWidget_2_itemClicked(QListWidgetItem *item);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_downloadButton_clicked();

    void on_stopButton_server_clicked();


    void on_pauseButton_server_clicked();

private:
    Ui::MainWindow *ui;
    bool _server;
    bool _client;
    QString _ipaddr;
    bool _playing = true;
    BufferManager _bm;

};

#endif // MAINWINDOW_H
