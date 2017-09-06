#ifndef DVIDBRANCHDIALOG_H
#define DVIDBRANCHDIALOG_H

#include <QDialog>

namespace Ui {
class DvidBranchDialog;
}

class DvidBranchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DvidBranchDialog(QWidget *parent = 0);
    ~DvidBranchDialog();

private:
    Ui::DvidBranchDialog *ui;
};

#endif // DVIDBRANCHDIALOG_H
