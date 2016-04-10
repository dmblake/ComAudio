#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    //void on_playbackButton_clicked();

    void on_playButton_clicked();
    void on_stopButton_clicked();
    void on_pauseButton_clicked();
private:
    Ui::MainWindow *ui;
    bool _server;
    bool _client;
    QString _ipaddr;
};

#endif // MAINWINDOW_H
