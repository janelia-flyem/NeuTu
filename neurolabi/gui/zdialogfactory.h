#ifndef ZDIALOGFACTORY_H
#define ZDIALOGFACTORY_H

#include <QObject>
#include <QDialog>
#include <zdviddialog.h>
#include <dvidimagedialog.h>
#include <zspinboxdialog.h>

class QSpacerItem;

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

private:
  QWidget *m_parentWidget;

};

#endif // ZDIALOGFACTORY_H
