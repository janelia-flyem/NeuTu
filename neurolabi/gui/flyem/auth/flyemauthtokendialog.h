#ifndef FLYEMAUTHTOKENDIALOG_H
#define FLYEMAUTHTOKENDIALOG_H

#include <QDialog>

#include "flyemauthtokenhandler.h"

namespace Ui {
class FlyEmAuthTokenDialog;
}

class FlyEmAuthTokenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FlyEmAuthTokenDialog(QWidget *parent = nullptr);
    ~FlyEmAuthTokenDialog();

signals:
    void requestUpdateAuthIcon();

private slots:
    void onLoginButton();
    void onCopyLoginUrlButton();
    void onTokenButton();
    void onCopyTokenUrlButton();
    void onSaveTokenButton();
    void onChangeServerButton();

private:
    Ui::FlyEmAuthTokenDialog *ui;

    FlyEmAuthTokenHandler m_handler;

    void updateServerLabel();
    void updateTokenText();
};

#endif // FLYEMAUTHTOKENDIALOG_H
