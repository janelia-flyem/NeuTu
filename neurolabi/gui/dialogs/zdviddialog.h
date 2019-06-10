#ifndef ZDVIDDIALOG_H
#define ZDVIDDIALOG_H

#include <QDialog>
#include <string>

#include "zdvidtargetproviderdialog.h"
#include "dvid/zdvidtarget.h"

namespace Ui {
class ZDvidDialog;
}

class StringListDialog;
class ZDvidAdvancedDialog;
class QLabel;
class QLineEdit;

class ZDvidDialog : public ZDvidTargetProviderDialog
{
  Q_OBJECT

public:
  explicit ZDvidDialog(QWidget *parent = 0);
  ~ZDvidDialog();


  ZDvidTarget& getDvidTarget();
  const ZDvidTarget& getDvidTarget(const std::string &name) const;

private slots:

  void setServer(int index);
  void saveCurrentTarget();
  void saveCurrentTargetAs();
  void deleteCurrentTarget();
  void editRoiList();
  void updateWidgetForDefaultSetting();
  void setAdvanced();
  void load();

private:
  bool usingDefaultSetting() const;
  void resetAdvancedDlg(const ZDvidTarget &dvidTarget);
  std::string getBodyLabelName() const;
  std::string getGrayscaleName() const;
  std::string getTileName() const;
  std::string getSegmentationName() const;
  std::string getSynapseName() const;
  std::string getRoiName() const;
  void setServer(const ZDvidTarget &dvidTarget, int index);

  int getPort() const;
  QString getAddress() const;
  QString getUuid() const;

  void addDvidTarget(ZDvidTarget &target);

  bool hasNameConflict(const std::string &name) const;
  void saveCurrentTarget(bool cloning);

private:
  Ui::ZDvidDialog *ui;
  QList<ZDvidTarget> m_dvidRepo;
  std::string m_customString;
  StringListDialog *m_roiDlg;
  ZDvidAdvancedDialog *m_advancedDlg;
  ZDvidTarget m_emptyTarget;
  const static char *m_dvidRepoKey;
};

#endif // ZDVIDDIALOG_H
