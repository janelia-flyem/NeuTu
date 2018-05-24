#ifndef SYNAPSEPREDICTIONINPUTDIALOG_H
#define SYNAPSEPREDICTIONINPUTDIALOG_H

#include <QDialog>
#include <QPalette>

#include "zintcuboid.h"

namespace Ui {
class SynapsePredictionInputDialog;
}

class SynapsePredictionInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SynapsePredictionInputDialog(QWidget *parent = 0);
    ~SynapsePredictionInputDialog();
    ZIntCuboid getVolume();
    void setVolume(ZIntCuboid volume);
    QString getRoI();
    void setRoI(QString roi);

private:
    Ui::SynapsePredictionInputDialog *ui;

};

#endif // SYNAPSEPREDICTIONINPUTDIALOG_H
