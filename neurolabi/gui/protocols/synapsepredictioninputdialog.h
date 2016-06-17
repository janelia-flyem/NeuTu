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

private slots:
    void xRangeChanged(int x);
    void yRangeChanged(int y);
    void zRangeChanged(int z);

private:
    Ui::SynapsePredictionInputDialog *ui;

    QPalette m_defaultPalette;
    QPalette m_redbgPalette;

    bool xValid();
    bool yValid();
    bool zValid();
    bool isValid();
    void accept();

};

#endif // SYNAPSEPREDICTIONINPUTDIALOG_H
