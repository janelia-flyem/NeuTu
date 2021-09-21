#ifndef ZDIALOGFACTORY_H
#define ZDIALOGFACTORY_H

#include <QObject>
#include <QDialog>
#include <QVector>
#include <QPointer>

#include "dialogs/zdviddialog.h"
#include "dialogs/zdvidtargetproviderdialog.h"
#include "dialogs/dvidimagedialog.h"
#include "dialogs/zspinboxdialog.h"
#include "dialogs/zspinboxgroupdialog.h"


class ZIntPoint;
class QSpacerItem;
class ZParameterArray;
class ZWidgetMessage;

#ifdef _WIN32
#undef GetOpenFileName
#undef GetSaveFileName
#endif

class ZDialogFactory
{
public:
  ZDialogFactory(QWidget *parentWidget);
  ~ZDialogFactory();

  enum ZDvidDialogType {
      DEFAULT,
      ORIGINAL,
      BRANCH_BROWSER
  };

  static ZDvidTargetProviderDialog* makeDvidDialog(QWidget *parent = 0, ZDvidDialogType type = DEFAULT);
  static QDialog* makeTestDialog(QWidget *parent = 0);
//  static QDialog* makeStackDialog(QWidget *parent = 0);
  static DvidImageDialog *makeDvidImageDialog(
      ZDvidTargetProviderDialog *dvidDlg, QWidget *parent = 0);
  static ZSpinBoxDialog *makeSpinBoxDialog(QWidget *parent = 0);
  static ZSpinBoxGroupDialog *makeDownsampleDialog(QWidget *parent);
  static QDialog* makeParameterDialog(
      const ZParameterArray &parameterArray,
      QWidget *parent);
  static bool Ask(const QString &title, const QString &msg, QWidget *parent);
  static void Warn(const QString &title, const QString &msg, QWidget *parent);
  static void Error(const QString &title, const QString &msg, QWidget *parent);
  static void Error(
      const QString &title, const QString &msg, const QString &detail,
      QWidget *parent);
  static void Info(const QString &title, const QString &msg, QWidget *parent);

  static bool WarningAskForContinue(
      const QString &title, const QString &msg, QWidget *parent);

  static QString GetDirectory(
      const QString &caption, const QString &filePath, QWidget *parent);
  static QString GetOpenFileName(
      const QString &caption, const QString &filePath, QWidget *parent);
  static QString GetSaveFileName(
      const QString &caption, const QString &filePath, QWidget *parent);
  static QString GetSaveFileName(
      const QString &caption, const QString &filePath, const QString &filter,
      QWidget *parent);

  static void Notify3DDisabled(QWidget *parent);

  static void About(QWidget *parent);

  static void PromptMessage(const ZWidgetMessage &msg, QWidget *parent);

  static ZIntPoint AskForIntPoint(QWidget *parent);
  static ZIntPoint AskForIntPoint(
      const ZIntPoint &defaultPos, QWidget *parent);

  static uint64_t GetUint64(
      const QString &title, const QString &label, QWidget *parent);

private:

  QWidget *m_parentWidget;
  static QString m_currentOpenFileName;
  static QString m_currentSaveFileName;
  static QString m_currentDirectory;
};

#endif // ZDIALOGFACTORY_H
