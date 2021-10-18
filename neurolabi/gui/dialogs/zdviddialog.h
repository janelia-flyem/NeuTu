#ifndef ZDVIDDIALOG_H
#define ZDVIDDIALOG_H

#include <QDialog>
#include <string>
#include <functional>

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
  explicit ZDvidDialog(QWidget *parent = nullptr);
  ~ZDvidDialog();


  ZDvidTarget& getDvidTarget();
  const ZDvidTarget& getDvidTarget(const std::string &name) const;

  void forEachTarget(std::function<void(const ZDvidTarget&)> f) const;

private slots:

  void setServer(int index);
  void saveCurrentTarget();
  void saveCurrentTargetAs();
  void deleteCurrentTarget();
  void editRoiList();
  void updateWidgetForDefaultSetting();
  void updateWidgetForDefaultSetting(const ZDvidTarget &target);
  void setAdvanced();
  void load();
  void exportTarget();

private:
  bool usingDefaultSetting() const;
  void resetAdvancedDlg(const ZDvidTarget &dvidTarget);
  std::string getBodyLabelName() const;
  std::string getGrayscaleName() const;
  std::string getTileName() const;
  std::string getSegmentationName() const;
  std::string getSynapseName() const;
  std::string getRoiName() const;
  void setServer(const ZDvidTarget &dvidTarget);
  ZDvidTarget getDvidTargetWithOriginalData();

  int getPort() const;
  QString getAddress() const;
  QString getUuid() const;

  bool addDvidTarget(ZDvidTarget &target);
  bool inputTargetName(ZDvidTarget &target);

  bool hasNameConflict(const std::string &name) const;
  void saveCurrentTarget(bool cloning);

  void updateWidgetValue(const ZDvidTarget &dvidTarget);
  void updateWidgetState(const ZDvidTarget &target);

  void initDvidRepo();
  void updateLastItemIcon(const ZDvidTarget &target);
  void addTargetItem(const ZDvidTarget &target);
  void updateAdvancedInfo();

public:
  struct PrivateTest {
    PrivateTest(ZDvidDialog *dlg): m_this(dlg) {
    }

    bool hasNameConflict(const std::string &name) const {
      return m_this->hasNameConflict(name);
    }

    void setSever(int index) {
      m_this->setServer(index);
    }

  private:
    ZDvidDialog *m_this;
  };

private:
  Ui::ZDvidDialog *ui;
  QList<ZDvidTarget> m_dvidRepo;
  std::string m_customString;
  StringListDialog *m_roiDlg;
  ZDvidAdvancedDialog *m_advancedDlg;
  ZDvidTarget m_emptyTarget;
  ZDvidTarget m_currentTarget;

  QString m_defaultSegmentationLabel;
  QString m_defaultGrayscaleLabel;
  QString m_defaultSynapseLabel;
  ZJsonObject m_currentDefaultSettings;

public:
  const static char *DVID_REPO_KEY;
  const static char *UNTITTLED_NAME;
};

#endif // ZDVIDDIALOG_H
