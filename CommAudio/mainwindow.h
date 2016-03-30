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
    ~MainWindow();
    void printToListView(std::string msg);

private slots:
    void on_startServerButton_clicked();

    void on_startMulticastServer_clicked();

    void on_startClientButton_clicked();

    void on_startClientMulticast_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
