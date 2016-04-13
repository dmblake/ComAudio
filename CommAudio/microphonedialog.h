#ifndef MICROPHONEDIALOG_H
#define MICROPHONEDIALOG_H

#include <QDialog>
#include <QBuffer>
#include "shared.h"

namespace Ui {
class MicrophoneDialog;
}

class MicrophoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MicrophoneDialog(QWidget *parent = 0);
    ~MicrophoneDialog();

private slots:
    void on_startButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::MicrophoneDialog *ui;
//    QAudioRecorder *audioRecorder;
//    QAudioProbe *probe;
};

#endif // MICROPHONEDIALOG_H
