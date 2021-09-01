#ifndef TASKBODYCLEAVE_H
#define TASKBODYCLEAVE_H

#include <set>

#include <QObject>
#include <QTime>
#include <QVector>
#include <QMutex>

#include "protocols/taskprotocoltask.h"
#include "zjsonarray.h"
#include "geometry/zpoint.h"
#include "zglmutils.h"

class ZDvidReader;
class ZDvidWriter;
class ZFlyEmBody3dDoc;
class ZFlyEmSupervisor;
class ZMesh;
class QAction;
class QCheckBox;
class QComboBox;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QRadioButton;
class QShortcut;

class TaskBodyCleave : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskBodyCleave(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);
  ~TaskBodyCleave() override;

  // For use with TaskProtocolTaskFactory.
  static QString taskTypeStatic(); //#Review-TZ: Use StaticFunc naming conversion for static functions
  static TaskBodyCleave* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);
  static QString menuLabelCreateFromGuiBodyId();
  static QJsonArray createFromGuiBodyId(ZFlyEmBody3dDoc *bodyDoc);
  static QString menuLabelCreateFromGui3dPoint();
  static QJsonArray createFromGui3dPoint(ZFlyEmBody3dDoc *bodyDoc);

  QString taskType() const override;
  QString actionString() override;
  QString targetString() override;
  bool skip(QString &reason) override;

  virtual void beforeNext() override;
  virtual void beforePrev() override;
  virtual void beforeLoading() override;
  virtual void onLoaded() override;
  virtual void beforeDone() override;

  virtual QWidget *getTaskWidget() override;
  virtual QMenu *getTaskMenu() override;

  uint64_t getBodyId() const;

  ProtocolTaskConfig getTaskConfig() const override;
  bool allowingSplit(uint64_t bodyId) const override;

  QString getInfo() const override;
  QString getError() const override;

public:
  //Temporary solution for resolving shortcut conflict
  void disableCleavingShortcut();
  void enableCleavingShortcut();
  void setCleavingShortcutEnabled(bool on);

private slots:
  void onShowCleavingChanged(int state);
  void onToggleShowCleaving();
  void onShowSeedsOnlyChanged(int state);
  void onToggleShowSeedsOnly();
  void onCleaveIndexShortcut();
  void onCleaveIndexChanged(int comboBoxIndex);
  void onSelectBody();
  void onShowBodyChanged(int state);
  void onToggleInChosenCleaveBody();
  void onAddToChosenCleaveBody();
  void updateChosenCleaveBody(bool toggle);
  void onToggleShowChosenCleaveBody();
  void onHideSelected();
  void onClearHidden();
  void onChooseCleaveMethod();
  void onColorByChanged();
  void onSpecifyAggloColor();

  void onNetworkReplyFinished(QNetworkReply *reply);

private:
  bool cleaveVerified(
      const std::map<std::size_t, std::vector<uint64_t> > &cleaveIndexToMeshIds,
      size_t indexNotCleavedOff) const;
  size_t getSupervoxelSize(uint64_t svId) const;
  void boostSupervoxelSizeRetrieval(
      const std::map<std::size_t, std::vector<uint64_t> > &cleaveIndexToMeshIds,
      size_t indexNotCleavedOff)
  const;

  QString getCleaveServer() const;
  QString getCleaveServerApi() const;
  static QString GetCleaveServerApi(const QString &server);

  void aggloIndexReplyFinished(QNetworkReply *reply);
  void cleaveServerReplyFinished(QNetworkReply *reply);

private:
  ZFlyEmBody3dDoc *m_bodyDoc;
  uint64_t m_bodyId;
  ZPoint m_bodyPt;
  int m_maxLevel;

  QWidget *m_widget;
  QCheckBox *m_showCleavingCheckBox;
  QComboBox *m_cleaveIndexComboBox;
  QPushButton *m_selectBodyButton;
  QCheckBox* m_showBodyCheckBox;
  QCheckBox *m_showSeedsOnlyCheckBox;
  QLabel *m_cleavingStatusLabel;
  QShortcut *m_shortcutToggle;
  QMenu *m_menu;
  QAction *m_showSeedsOnlyAction;
  QAction *m_addToBodyAction;
  QAction *m_toggleInBodyAction;
  QAction *m_toggleShowChosenCleaveBodyAction;
  std::map<QAction *, int> m_actionToComboBoxIndex;
  QRadioButton *m_colorSupervoxelsButton;
  QRadioButton *m_colorAggloButton;
  QPushButton *m_specifyAggloColorButton;

  bool m_skip = false;
  int m_timeOfLastSkipCheck = -1;

  ZFlyEmSupervisor *m_supervisor;
  bool m_checkedOut = false;

  QTime m_usageTimer;
  std::vector<int> m_usageTimes;
  std::vector<int> m_startupTimes;

  // The cleave index assignments created by the last cleaving operation (initially empty).
  std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndex;

  // The cleave index assignments specified by the user, to be used as seeds for the next
  // cleaving operation.
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndex;

  std::set<size_t> m_hiddenCleaveIndices;

  QString m_cleaveMethod;

  QNetworkAccessManager *m_networkManager;
  std::size_t m_cleaveRepliesPending = 0;

  // The latest cleave server reply that was applied is saved for debugging purposes.
  QJsonObject m_cleaveReply;

  class CleaveCommand;

  std::size_t chosenCleaveIndex() const;

  std::set<uint64_t> m_hiddenIds;

  std::map<std::uint64_t, std::size_t> m_meshIdToAggloIndex;

  void buildTaskWidget();
  void updateColors();
  void updateMeshIdToAggloIndex();
  bool loadColorsAgglo(QString filepath);
  void setDefaultColorsAgglo();
  void updateColorsAgglo();

  bool uiIsEnabled() const;

  void bodiesForCleaveIndex(std::set<uint64_t>& result, std::size_t cleaveIndex,
                            bool ignoreSeedsOnly = false);

  void selectBodies(const std::set<uint64_t>& bodies, bool select = true);

  void applyPerTaskSettings();
  void applyColorMode(bool showingCleaving, bool colorBySupervoxels = true);
  void enableCleavingUI(bool showingCleaving);

  void cleave(unsigned int requestNumber);

  bool getUnassignedMeshes(std::vector<uint64_t> &result) const;

  void updateVisibility();

  std::set<std::size_t> hiddenChanges(const std::map<uint64_t, std::size_t>& newMeshIdToCleaveIndex) const;
  void showHiddenChangeWarning(const std::set<std::size_t>& hiddenChangedIndices);

  bool showCleaveReplyWarnings(const QJsonObject& reply);
  bool showCleaveReplyOmittedMeshes(std::map<uint64_t, std::size_t> meshIdToCleaveIndex);
  void displayWarning(const QString& title, const QString& text,
                      const QString& details = "",
                      bool allowSuppression = false);

  std::size_t getIndexNotCleavedOff() const;

  bool writeOutput(ZDvidWriter &writer,
                   const std::map<std::size_t, std::vector<uint64_t>> &cleaveIndexToMeshIds,
                   const std::size_t &indexNotCleavedOff,
                   std::vector<QString> &responseLabels,
                   std::vector<uint64_t> &mutationIds);
  void writeAuxiliaryOutput(const ZDvidReader &reader, ZDvidWriter &writer,
                            const std::map<std::size_t, std::vector<uint64_t>> &cleaveIndexToMeshIds = {},
                            const std::vector<QString> &newBodyIds = {},
                            const std::vector<uint64_t> &mutationIds = {});
  ZJsonArray readAuxiliaryOutput(const ZDvidReader& reader) const;

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual bool allowCompletion() override;
  virtual void onCompleted() override;
};

#endif // TASKBODYCLEAVE_H
