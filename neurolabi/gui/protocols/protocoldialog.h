#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>

#include "zjsonobject.h"

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
    std::string getName();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onFirstButton();
    void onDoButton();
    void onSkipButton();
    void onExitButton();
    void onCompleteButton();

private:
    static const std::string PROTOCOL_NAME;
    static const std::string KEY_PENDING;
    static const std::string KEY_FINISHED;

    Ui::ProtocolDialog *ui;
    QStringList m_pendingList;
    QStringList m_finishedList;
    QString m_currentItem;

    void saveState();
    void updateLabels();
    void gotoNextItem();
};

#endif // PROTOCOLDIALOG_H
