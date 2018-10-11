#ifndef TASKBODYHISTORY_H
#define TASKBODYHISTORY_H

#include "protocols/taskprotocoltask.h"
#include <QObject>

class ZFlyEmBody3dDoc;
class QSlider;

class TaskBodyHistory : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskBodyHistory(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  // For use with TaskProtocolTaskFactory.
  static QString taskTypeStatic();
  static TaskBodyHistory* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  QString taskType() const override;
  QString actionString() override;
  QString targetString() override;

  virtual QWidget *getTaskWidget() override;

private slots:
  void updateLevel(int level);

private:
  ZFlyEmBody3dDoc *m_bodyDoc;
  uint64_t m_bodyId;
  int m_maxLevel;
  QSlider *m_widget;

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual void onCompleted() override;

};

#endif // TASKBODYHISTORY_H
