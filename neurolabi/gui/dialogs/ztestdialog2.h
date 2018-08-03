#ifndef ZTESTDIALOG2_H
#define ZTESTDIALOG2_H

#include <QDialog>
#include <QFileDialog>

#include "flyembodyinfodialog.h"

namespace Ui {
class ZTestDialog2;
}

class ZTestDialog2 : public QDialog
{
    Q_OBJECT

public:
    explicit ZTestDialog2(QWidget *parent = 0);
    ~ZTestDialog2();

private slots:
    void onBrowseButton();
    void onBodyInfoButton();

private:
    Ui::ZTestDialog2 *ui;

    FlyEmBodyInfoDialog *m_flyEmBodyInfoDialog;

};

#endif // ZTESTDIALOG2_H
