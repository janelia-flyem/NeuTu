#ifndef ZDVIDDIALOG_H
#define ZDVIDDIALOG_H

#include <QDialog>
#include <string>
#include "dvid/zdvidtarget.h"

namespace Ui {
class ZDvidDialog;
}

class StringListDialog;

class ZDvidDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidDialog(QWidget *parent = 0);
  ~ZDvidDialog();

 //void loadConfig(const std::string &filePath);

  int getPort() const;
  QString getAddress() const;
  QString getUuid() const;

  ZDvidTarget& getDvidTarget();

  void addDvidTarget(ZDvidTarget &target);

  bool hasNameConflict(const std::string &name) const;
  void saveCurrentTarget(bool cloning);

public slots:
  void setServer(int index);
  void saveCurrentTarget();
  void saveCurrentTargetAs();
  void deleteCurrentTarget();
  void editRoiList();

private:
  Ui::ZDvidDialog *ui;
  QList<ZDvidTarget> m_dvidRepo;
  std::string m_customString;
  StringListDialog *m_roiDlg;
  const static char *m_dvidRepoKey;
};

#endif // ZDVIDDIALOG_H
