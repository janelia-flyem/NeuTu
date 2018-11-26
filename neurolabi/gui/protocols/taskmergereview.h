#ifndef TASKMERGEREVIEW_H
#define TASKMERGEREVIEW_H

#include "protocols/taskprotocoltask.h"
#include "zpoint.h"
#include <QObject>
#include <QTime>
#include <QVector>
#include <set>

class ZFlyEmBody3dDoc;
class ZMesh;
class QAction;
class QCheckBox;
class QPushButton;
class QRadioButton;
class QShortcut;

class TaskMergeReview : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskMergeReview(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  // For use with TaskProtocolTaskFactory.
  static QString taskTypeStatic();
  static TaskMergeReview* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  virtual QString taskType() const override;
  virtual QString actionString() override;
  virtual QString targetString() override;

  virtual bool skip() override;
  virtual void beforeNext() override;
  virtual void beforePrev() override;
  virtual void beforeDone() override;

  virtual QWidget *getTaskWidget() override;
  virtual QMenu *getTaskMenu() override;
  virtual bool usePrefetching() override;

  const std::set<uint64_t>& getBodyIds() const;

private slots:
  void onCycleAnswer();
  void onButtonToggled();
  void onSelectCurrentBody();
  void onNextBodyToSelect();
  void onPrevBodyToSelect();
  void onShowMajorChanged(int state);
  void onShowMinorChanged(int state);
  void onShowSupervoxelsChanged(int state);
  void onToggleShowSupervoxels();
  void zoomOutToShowAll();
  void onHideSelected();
  void onClearHidden();
  void onToggleIsolation();

private:
  ZFlyEmBody3dDoc *m_bodyDoc;
  QString m_taskId;
  std::set<uint64_t> m_superVoxelIds;
  std::set<uint64_t> m_majorSuperVoxelIds;
  std::set<uint64_t> m_bodyIds;
  std::set<uint64_t> m_majorBodyIds;

  std::map<uint64_t, std::size_t> m_bodyIdToColorIndex;
  std::set<uint64_t>::const_iterator m_bodyToSelect;
  std::size_t m_bodyToSelectIndex;

  enum class Skip { NOT_SKIPPED, SKIPPED_MAPPING, SKIPPED_SIZES, SKIPPED_MESHES, SKIPPED_MAJOR };
  Skip m_skip = Skip::NOT_SKIPPED;
  int m_timeOfLastSkipCheck = -1;

  QTime m_usageTimer;
  std::vector<int> m_usageTimes;

  QWidget *m_widget;
  QRadioButton *m_mergeButton;
  QRadioButton *m_mergeMajorButton;
  QRadioButton *m_dontMergeButton;
  QRadioButton *m_processFurtherButton;
  QRadioButton *m_irrelevantButton;
  QRadioButton *m_dontKnowButton;
  QRadioButton *m_lastSavedButton;
  QPushButton *m_selectCurrentBodyButton;
  QPushButton *m_nextBodyToSelectButton;
  QPushButton *m_prevBodyToSelectButton;
  QCheckBox *m_showMajorCheckBox;
  QCheckBox *m_showMinorCheckBox;
  QCheckBox *m_showSupervoxelsCheckBox;
  QMenu *m_menu;

  std::set<QString> m_warningTextToSuppress;

  std::set<uint64_t> m_hiddenIds;

  enum class SetBodiesResult { SUCCEEDED, FAILED_MAPPING, FAILED_SIZES, FAILED_MAJOR };
  SetBodiesResult setBodiesFromSuperVoxels();
  void buildTaskWidget();
  void updateColors();

  bool bodyToSelectIsFilteredOut() const;
  void incrBodyToSelect();
  void decrBodyToSelect();
  void updateSelectCurrentBodyButton();

  void selectBodies(const std::set<uint64_t>& bodies, bool toSelect = true);

  void applyPerTaskSettings();
  void applyColorMode(bool showingCleaving);

  void updateVisibility();
  void zoomToFitMeshes();

  void displayWarning(const QString& title, const QString& text,
                      const QString& details = "",
                      bool allowSuppression = false);

  std::string outputKey() const;
  void writeOutput();

  QString readResult() const;
  void restoreResult(QString result);

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual void onLoaded() override;
  virtual bool allowCompletion() override;
  virtual void onCompleted() override;

  virtual ProtocolTaskConfig getTaskConfig() const override;
};

#endif // TASKMERGEREVIEW_H
