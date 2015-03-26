#ifndef ZDIALOGFACTORY_H
#define ZDIALOGFACTORY_H

#include <QObject>
#include <QDialog>
#include <QVector>
#include <QPointer>
#include "zdviddialog.h"
#include "dvidimagedialog.h"
#include "zspinboxdialog.h"
#include "zparameter.h"
#include"zspinboxgroupdialog.h"

class QSpacerItem;
class ZParameterArray;

class ZDialogFactory
{
public:
  ZDialogFactory(QWidget *parentWidget);
  ~ZDialogFactory();

  static ZDvidDialog* makeDvidDialog(QWidget *parent = 0);
  static QDialog* makeTestDialog(QWidget *parent = 0);
  static QDialog* makeStackDialog(QWidget *parent = 0);
  static DvidImageDialog *makeDvidImageDialog(
      ZDvidDialog *dvidDlg, QWidget *parent = 0);
  static ZSpinBoxDialog *makeSpinBoxDialog(QWidget *parent = 0);
  static ZSpinBoxGroupDialog *makeDownsampleDialog(QWidget *parent);
  static QDialog* makeParameterDialog(
      const ZParameterArray &parameterArray,
      QWidget *parent);
  static bool ask(const QString &title, const QString &msg, QWidget *parent);

  static QString GetDirectory(
      const QString &caption, const QString &filePath, QWidget *parent);
  static QString GetFileName(
      const QString &caption, const QString &filePath, QWidget *parent);

private:
  QWidget *m_parentWidget;

};

#endif // ZDIALOGFACTORY_H
