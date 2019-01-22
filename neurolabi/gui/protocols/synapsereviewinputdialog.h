#ifndef SYNAPSEREVIEWINPUTDIALOG_H
#define SYNAPSEREVIEWINPUTDIALOG_H

#include <QDialog>

#include "geometry/zintcuboid.h"

namespace Ui {
class SynapseReviewInputDialog;
}

class SynapseReviewInputDialog : public QDialog
{
    Q_OBJECT

public:
    enum SynapseReviewInputOptions {
        BY_BODYID,
        BY_VOLUME
    };

    explicit SynapseReviewInputDialog(QWidget *parent = 0);
    ~SynapseReviewInputDialog();
    SynapseReviewInputDialog::SynapseReviewInputOptions getInputOption();
    QString getBodyID();
    ZIntCuboid getVolume();

private:
    Ui::SynapseReviewInputDialog *ui;
};

#endif // SYNAPSEREVIEWINPUTDIALOG_H
