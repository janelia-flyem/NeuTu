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

private:
    enum Status {
        PROTOCOL_COMPLETE,
        PROTOCOL_INCOMPLETE
    };

    Ui::ProtocolDialog *ui;
    QStringList m_pendingList;
    QStringList m_finishedList;
    Status m_protocolStatus;
    QString m_currentItem;

    void saveState();
    void updateLabels();
    void gotoNextItem();
};

#endif // PROTOCOLDIALOG_H
