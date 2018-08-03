#ifndef ZTESTDIALOG2_H
#define ZTESTDIALOG2_H

#include <QDialog>
#include <QFileDialog>

#include "dvidbranchdialog.h"

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
    void onOpenDvidButton();

private:
    Ui::ZTestDialog2 *ui;

    DvidBranchDialog * m_dialog;

};

#endif // ZTESTDIALOG2_H
