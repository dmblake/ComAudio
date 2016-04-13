#ifndef MICROPHONEDIALOG_H
#define MICROPHONEDIALOG_H

#include <QDialog>
#include <QAudioRecorder>
#include <QAudioProbe>

namespace Ui {
class MicrophoneDialog;
}

class MicrophoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MicrophoneDialog(QWidget *parent = 0);
    ~MicrophoneDialog();

private:
    Ui::MicrophoneDialog *ui;
    QAudioRecorder *audioRecorder;
    QAudioProbe *probe;
};

#endif // MICROPHONEDIALOG_H
