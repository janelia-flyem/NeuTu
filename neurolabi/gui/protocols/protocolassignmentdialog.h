#ifndef PROTOCOLASSIGNMENTDIALOG_H
#define PROTOCOLASSIGNMENTDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

#include "protocolassignmentclient.h"

namespace Ui {
class ProtocolAssignmentDialog;
}

class ProtocolAssignmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolAssignmentDialog(QWidget *parent = nullptr);
    ~ProtocolAssignmentDialog();

private slots:
    void onLoadStartedButton();
    void onGetNewButton();


private:
    Ui::ProtocolAssignmentDialog *ui;

    enum TableColumns {
        ID_COLUMN,
        PROJECT_COLUMN,
        PROTOCOL_COLUMN,
        NAME_COLUMN,
        STARTED_COLUMN
    };

    QStandardItemModel * m_model;
    ProtocolAssignmentClient m_client;
    QString m_token;
    QString m_username;

    bool checkForTokens();
    void updateStartedTable();
    void clearStartedTable();

    void setHeaders(QStandardItemModel *model);

    void showError(QString title, QString message);
    void showMessage(QString title, QString message);
};

#endif // PROTOCOLASSIGNMENTDIALOG_H
