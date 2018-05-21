#ifndef TASKBODYMERGE_H
#define TASKBODYMERGE_H

#include "protocols/taskprotocoltask.h"
#include "zpoint.h"
#include <QObject>

class ZFlyEmBody3dDoc;
class QRadioButton;

class TaskBodyMerge : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskBodyMerge(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);
  QString tasktype() override;
  QString actionString() override;
  QString targetString() override;

  virtual void beforeDone() override;

  virtual QWidget *getTaskWidget() override;
  virtual QMenu *getTaskMenu() override;

private slots:
  void onCycleAnswer();
  void onButtonToggled();

private:
  ZFlyEmBody3dDoc *m_bodyDoc;
  uint64_t m_supervoxelId1;
  uint64_t m_supervoxelId2;
  uint64_t m_bodyId1;
  uint64_t m_bodyId2;
  ZPoint m_supervoxelPoint1;
  ZPoint m_supervoxelPoint2;

  QWidget *m_widget;
  QRadioButton *m_mergeButton;
  QRadioButton *m_dontMergeButton;
  QRadioButton *m_dontKnowButton;
  QMenu *m_menu;

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual void onLoaded() override;
  virtual void onCompleted() override;

  void setBodiesFromSuperVoxels();
  void buildTaskWidget();
  void applyColorMode(bool merging);
  void updateColors();

  ZPoint mergePosition() const;
  void zoomToMergePosition();
};

#endif // TASKBODYMERGE_H
