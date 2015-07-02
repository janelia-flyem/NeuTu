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
  static bool Ask(const QString &title, const QString &msg, QWidget *parent);

  static QString GetDirectory(
      const QString &caption, const QString &filePath, QWidget *parent);
  static QString GetOpenFileName(
      const QString &caption, const QString &filePath, QWidget *parent);
  static QString GetSaveFileName(
      const QString &caption, const QString &filePath, QWidget *parent);

  static void Notify3DDisabled(QWidget *parent);

private:
  QWidget *m_parentWidget;
  static QString m_currentOpenFileName;
  static QString m_currentSaveFileName;
  static QString m_currentDirectory;
};

#endif // ZDIALOGFACTORY_H
