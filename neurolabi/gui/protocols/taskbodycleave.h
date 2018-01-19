#ifndef TASKBODYCLEAVE_H
#define TASKBODYCLEAVE_H

#include "protocols/taskprotocoltask.h"
#include <QObject>

#include <QVector>

class ZFlyEmBody3dDoc;
class QCheckBox;
class QComboBox;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QShortcut;

class QSlider;

class TaskBodyCleave : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskBodyCleave(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);
  QString tasktype() override;
  QString actionString() override;
  QString targetString() override;

  virtual void beforeNext() override;
  virtual void beforePrev() override;
  virtual void beforeDone() override;

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
  ZFlyEmBody3dDoc *m_bodyDoc;
  uint64_t m_bodyId;
  int m_maxLevel;

  QWidget *m_widget;
  QSlider *m_levelSlider;
  QCheckBox *m_showCleavingCheckBox;
  QComboBox *m_cleaveIndexComboBox;
  QPushButton *m_buttonAdd;
  QPushButton *m_buttonRemove;
  QPushButton *m_buttonCleave;
  QShortcut *m_shortcutToggle;

  // The cleave index assignments created by the last cleaving operation (initially empty).
  std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndex;

  // The cleave index assignments specified by the user, to be used as seeds for the next
  // cleaving operation.
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndex;

  QNetworkAccessManager *m_networkManager;

  class SetCleaveIndicesCommand;
  class CleaveCommand;

  std::size_t chosenCleaveIndex() const;

  void buildTaskWidget();
  void updateColors();

  void applyPerTaskSettings();
  void applyColorMode(bool showingCleaving);
  void enableCleavingUI(bool showingCleaving);

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual void onCompleted() override;
};

#endif // TASKBODYCLEAVE_H
