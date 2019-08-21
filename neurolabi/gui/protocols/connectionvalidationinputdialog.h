#ifndef CONNECTIONVALIDATIONINPUTDIALOG_H
#define CONNECTIONVALIDATIONINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectionValidationInputDialog;
}

class ConnectionValidationInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionValidationInputDialog(QWidget *parent = 0);
    ~ConnectionValidationInputDialog();

private:
    Ui::ConnectionValidationInputDialog *ui;
};

#endif // CONNECTIONVALIDATIONINPUTDIALOG_H
