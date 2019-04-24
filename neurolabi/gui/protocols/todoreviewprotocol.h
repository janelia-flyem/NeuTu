#ifndef TODOREVIEWPROTOCOL_H
#define TODOREVIEWPROTOCOL_H

#include <QDialog>

#include "protocoldialog.h"
#include "zjsonobject.h"


namespace Ui {
class ToDoReviewProtocol;
}

class ToDoReviewProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit ToDoReviewProtocol(QWidget *parent = 0);
    ~ToDoReviewProtocol();   
    bool initialize();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onExitButton();
    void onCompleteButton();

private:
    static const std::string KEY_VERSION;
    static const int fileVersion;

    void saveState();


    Ui::ToDoReviewProtocol *ui;
};

#endif // TODOREVIEWPROTOCOL_H
