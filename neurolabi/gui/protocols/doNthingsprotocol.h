#ifndef DoNThingsProtocol_H
#define DoNThingsProtocol_H

#include <QDialog>

#include "zjsonobject.h"

#include "protocoldialog.h"

namespace Ui {
class DoNThingsProtocol;
}

class DoNThingsProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit DoNThingsProtocol(QWidget *parent = 0);
    ~DoNThingsProtocol();    
    bool initialize();

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
    static const std::string KEY_PENDING;
    static const std::string KEY_FINISHED;

    Ui::DoNThingsProtocol *ui;
    QStringList m_pendingList;
    QStringList m_finishedList;
    QString m_currentItem;

    void saveState();
    void updateLabels();
    void gotoNextItem();
};

#endif // DoNThingsProtocol_H
