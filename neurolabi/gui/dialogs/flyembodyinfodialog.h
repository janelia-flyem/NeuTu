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

public:
  void importBookmarksFile(const QString &filename);

signals:
  void bodyActivated(uint64_t bodyId);

private slots:
    void onOpenButton();
    void onCloseButton();
    void activateBody(QModelIndex modelIndex);

private:
    Ui::FlyEmBodyInfoDialog *ui;
    QStandardItemModel* m_model;
    QStandardItemModel* createModel(QObject*);
    void setHeaders(QStandardItemModel*);
    void updateModel(ZJsonValue object);
    bool isValidBookmarkFile(ZJsonObject object);
};

#endif // FLYEMBODYINFODIALOG_H
