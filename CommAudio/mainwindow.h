#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "shared.h"

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

private slots:

    void on_updateButton_clicked();
    void on_close_clicked();

    void on_playButton_server_clicked();

    void on_playButton_client_clicked();
    void on_refreshButton_clicked();

    void on_listWidget_2_itemClicked(QListWidgetItem *item);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_downloadButton_clicked();

    void on_microphoneButton_server_clicked();

    void on_microphoneButton_client_clicked();

private:
    Ui::MainWindow *ui;
    bool _server;
    bool _client;
    QString _ipaddr;

};

#endif // MAINWINDOW_H
