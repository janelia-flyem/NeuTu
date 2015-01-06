#ifndef ZDIALOGFACTORY_H
#define ZDIALOGFACTORY_H

#include <QObject>
#include <QDialog>
#include <QVector>
#include <QPointer>
#include <zdviddialog.h>
#include <dvidimagedialog.h>
#include <zspinboxdialog.h>
#include "zparameter.h"

class QSpacerItem;
class ZParameterArray;

class ZDialogFactory
{
public:
  ZDialogFactory(QWidget *parentWidget);
  ~ZDialogFactory();

  static ZDvidDialog* makeDvidDialog(QWidget *parent = 0);
  static QDialog* makeTestDialog(QWidget *parent = 0);
  static DvidImageDialog *makeDvidImageDialog(
      ZDvidDialog *dvidDlg, QWidget *parent = 0);
  static ZSpinBoxDialog *makeSpinBoxDialog(QWidget *parent = 0);
  static QDialog* makeParameterDialog(
      const ZParameterArray &parameterArray,
      QWidget *parent);
  static bool ask(const QString &title, const QString &msg, QWidget *parent);

private:
  QWidget *m_parentWidget;

};

#endif // ZDIALOGFACTORY_H
