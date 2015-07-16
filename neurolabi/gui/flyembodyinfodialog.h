#ifndef FLYEMBODYINFODIALOG_H
#define FLYEMBODYINFODIALOG_H

#include <QDialog>
#include <QtGui>

#include "zjsonobject.h"

namespace Ui {
class FlyEmBodyInfoDialog;
}

class FlyEmBodyInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FlyEmBodyInfoDialog(QWidget *parent = 0);
    ~FlyEmBodyInfoDialog();

private slots:
    void onOpenButton();
    void onCloseButton();
private:
    Ui::FlyEmBodyInfoDialog *ui;
    QStandardItemModel* m_model;
    QStandardItemModel* createModel(QObject*);
    void updateModel();
    void importBookmarksFile(QString filename);
    bool isValidBookmarkFile(ZJsonObject object);
};

#endif // FLYEMBODYINFODIALOG_H
