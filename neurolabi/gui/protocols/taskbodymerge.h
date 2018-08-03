#ifndef TASKBODYMERGE_H
#define TASKBODYMERGE_H

#include "protocols/taskprotocoltask.h"
#include "zglmutils.h"
#include "zpoint.h"
#include <QObject>
#include <QTime>

class ZFlyEmBody3dDoc;
class QAction;
class QCheckBox;
class QRadioButton;

class TaskBodyMerge : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskBodyMerge(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);
  QString tasktype() override;
  QString actionString() override;
  QString targetString() override;

  virtual bool skip() override;
  virtual void beforeNext() override;
  virtual void beforePrev() override;
  virtual void beforeDone() override;

  virtual QWidget *getTaskWidget() override;
  virtual QMenu *getTaskMenu() override;
  virtual bool usePrefetching() override;

private slots:
  void onCycleAnswer();
  void onTriggerShowHiRes();
  void onButtonToggled();
  void onShowHiResStateChanged(int state);
  void zoomToMergePosition(bool justLoaded = false);
  void zoomOutToShowAll();

private:
  ZFlyEmBody3dDoc *m_bodyDoc;
  uint64_t m_supervoxelId1;
  uint64_t m_supervoxelId2;
  uint64_t m_bodyId1;
  uint64_t m_bodyId2;
  ZPoint m_supervoxelPoint1;
  ZPoint m_supervoxelPoint2;

  size_t m_hiResCount;

  QTime m_usageTimer;
  std::vector<int> m_usageTimes;
  std::vector<QString> m_resultHistory;

  size_t m_initialAngleMethod;
  glm::vec3 m_initialUp;

  QWidget *m_widget;
  QRadioButton *m_mergeButton;
  QRadioButton *m_dontMergeButton;
  QRadioButton *m_mergeLaterButton;
  QRadioButton *m_irrelevantButton;
  QRadioButton *m_dontKnowButton;
  QRadioButton *m_lastSavedButton;
  QCheckBox *m_showHiResCheckBox;
  QMenu *m_menu;
  QAction *m_showHiResAction;

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual void onLoaded() override;
  virtual void onCompleted() override;

  void setBodiesFromSuperVoxels();
  void buildTaskWidget();
  void applyColorMode(bool merging);
  void updateColors();
  ZPoint mergePosition() const;
  void initAngleForMergePosition(bool justLoaded);
  void zoomToMeshes(bool onlySmaller);
  void configureShowHiRes();
  void showBirdsEyeView(bool show);
  void writeResult();
  void writeResult(const QString &result);
  QString readResult();
  void restoreResult(const QString &result);
  void suggestWriting();
};

#endif // TASKBODYMERGE_H
