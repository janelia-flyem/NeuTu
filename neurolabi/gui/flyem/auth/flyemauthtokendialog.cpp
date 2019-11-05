#include "flyemauthtokendialog.h"
#include "ui_flyemauthtokendialog.h"

#include <QGuiApplication>
#include <QClipboard>

/*
 * this dialog helps users to log in to the Fly EM authentication service,
 * get their token, and paste it in for use in NeuTu
 */
FlyEmAuthTokenDialog::FlyEmAuthTokenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlyEmAuthTokenDialog)
{
    ui->setupUi(this);

    // ui connections
    connect(ui->openLoginButton, SIGNAL(clicked(bool)), this, SLOT(onLoginButton()));
    connect(ui->copyLoginURLButton, SIGNAL(clicked(bool)), this, SLOT(onCopyLoginUrlButton()));
    connect(ui->openTokenButton, SIGNAL(clicked(bool)), this, SLOT(onTokenButton()));
    connect(ui->copyTokenUrlButton, SIGNAL(clicked(bool)), this, SLOT(onCopyTokenUrlButton()));
    connect(ui->saveTokenButton, SIGNAL(clicked(bool)), this, SLOT(onSaveTokenButton()));

    updateToken();
    updateServerLabel(m_handler.getServer());

}

void FlyEmAuthTokenDialog::onLoginButton() {
    m_handler.openLoginInBrowser();
}

void FlyEmAuthTokenDialog::onTokenButton() {
    m_handler.openTokenInBrowser();
}

void FlyEmAuthTokenDialog::onCopyLoginUrlButton() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_handler.getLoginUrl());
}

void FlyEmAuthTokenDialog::onCopyTokenUrlButton() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_handler.getTokenUrl());
}

void FlyEmAuthTokenDialog::onSaveTokenButton() {
    QString token = ui->tokenText->toPlainText();
    if (!token.isEmpty()) {
        // note that the token in in json form at this point: {"token": "....."}
        m_handler.saveToken(token);
    }
}

void FlyEmAuthTokenDialog::updateServerLabel(QString server) {
    ui->serverLabel->setText("Current authentication server: " + server);
}

void FlyEmAuthTokenDialog::updateToken() {
    if (m_handler.hasToken()) {
        ui->tokenText->setText(m_handler.getToken());
    }
}

FlyEmAuthTokenDialog::~FlyEmAuthTokenDialog()
{
    delete ui;
}
