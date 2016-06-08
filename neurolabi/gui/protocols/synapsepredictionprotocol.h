#ifndef SYNAPSEPREDICTIONPROTOCOL_H
#define SYNAPSEPREDICTIONPROTOCOL_H

#include <QDialog>

#include "zjsonobject.h"

#include "protocoldialog.h"

namespace Ui {
class SynapsePredictionProtocol;
}

class SynapsePredictionProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit SynapsePredictionProtocol(QWidget *parent = 0);
    ~SynapsePredictionProtocol();
    bool initialize();
    std::string getName();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);


private slots:
    void onFirstButton();
    void onMarkedButton();
    void onSkipButton();
    void onGotoButton();
    void onExitButton();
    void onCompleteButton();

private:
    static const std::string PROTOCOL_NAME;
    static const std::string KEY_PENDING;
    static const std::string KEY_FINISHED;

    Ui::SynapsePredictionProtocol *ui;

    void saveState();
    void updateLabels();
    void gotoNextItem();

};

#endif // SYNAPSEPREDICTIONPROTOCOL_H
