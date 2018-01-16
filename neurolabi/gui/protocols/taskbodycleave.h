#ifndef TASKBODYCLEAVE_H
#define TASKBODYCLEAVE_H

#include "protocols/taskprotocoltask.h"
#include <QObject>

#include <QVector>

class ZFlyEmBody3dDoc;
class QComboBox;
class QNetworkAccessManager;
class QNetworkReply;

class QSlider;

class TaskBodyCleave : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskBodyCleave(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);
  QString tasktype() override;
  QString actionString() override;
  QString targetString() override;

  virtual QWidget *getTaskWidget() override;

private slots:
  void updateLevel(int level);

  void onShowCleavingChanged(bool show);
  void onChosenCleaveIndexChanged();
  void onAddToChosenCleaveBody();
  void onRemoveFromChosenCleaveBody();
  void onToggleInChosenCleaveBody();
  void onCleave();

  void onNetworkReplyFinished(QNetworkReply *reply);

private:
  QWidget *m_widget;
  QSlider *m_levelSlider;
  QComboBox *m_cleaveIndexComboBox;
  ZFlyEmBody3dDoc *m_bodyDoc;
  uint64_t m_bodyId;
  int m_maxLevel;

  // The cleave index assignments created by the last cleaving operation (initially empty).
  std::map<uint64_t, std::size_t> m_meshIdToLastCleaveIndex;

  // The cleave index assignments specified by the user, to be used for the next cleaving operation.
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndex;

  QNetworkAccessManager *m_networkManager;

  class SetCleaveIndicesCommand;
  class CleaveCommand;

  std::size_t chosenCleaveIndex() const;

  void buildTaskWidget();
  void updateColors();

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual void onCompleted() override;
};

#endif // TASKBODYCLEAVE_H
