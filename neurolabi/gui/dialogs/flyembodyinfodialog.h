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
  void dataChanged(ZJsonValue object);
  void jsonLoadError(QString message);

private slots:
    void onOpenButton();
    void onCloseButton();
    void activateBody(QModelIndex modelIndex);
    void updateModel(ZJsonValue object);
    void onJsonLoadError(QString message);

private:
    Ui::FlyEmBodyInfoDialog *ui;
    QStandardItemModel* m_model;
    QStandardItemModel* createModel(QObject*);
    void setHeaders(QStandardItemModel*);
    bool isValidBookmarkFile(ZJsonObject object);
};

#endif // FLYEMBODYINFODIALOG_H
