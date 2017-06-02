#ifndef SYNAPSEREVIEWINPUTDIALOG_H
#define SYNAPSEREVIEWINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class SynapseReviewInputDialog;
}

class SynapseReviewInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SynapseReviewInputDialog(QWidget *parent = 0);
    ~SynapseReviewInputDialog();
    SynapseReviewInputDialog::SynapseReviewInputOptions getInputOption();
    QString getRoI();

private:
    enum SynapseReviewInputOptions {
        BY_BODYID,
        BY_ROI,
        BY_VOLUME
    };


    Ui::SynapseReviewInputDialog *ui;
};

#endif // SYNAPSEREVIEWINPUTDIALOG_H
