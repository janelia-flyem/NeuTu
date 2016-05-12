#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>

namespace Ui {
class ProtocolDialog;
}

class ProtocolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolDialog(QWidget *parent = 0);
    ~ProtocolDialog();    
    virtual bool initialize();

signals:
    void protocolExiting();

private slots:
    void onFirstButton();
    void onDoButton();
    void onSkipButton();
    void onExitButton();
    void onCompleteButton();
    void onGotoButton();

private:
    Ui::ProtocolDialog *ui;
    QStringList m_pendingList;
    QStringList m_finishedList;
    void updateProgressLabel();
};

#endif // PROTOCOLDIALOG_H
