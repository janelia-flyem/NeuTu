#ifndef TASKBODYCLEAVE_H
#define TASKBODYCLEAVE_H

#include "protocols/taskprotocoltask.h"
#include <QObject>

#include <QVector>

class ZDvidReader;
class ZDvidWriter;
class ZFlyEmBody3dDoc;
class QAction;
class QCheckBox;
class QComboBox;
class QLabel;
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
  virtual QMenu *getTaskMenu() override;

private slots:
  void updateLevel(int level);

  void onShowCleavingChanged(int state);
  void onToggleShowCleaving();
  void onShowSeedsOnlyChanged(int state);
  void onToggleShowSeedsOnly();
  void onChosenCleaveIndexChanged();
  void onSelectBody();
  void onToggleInChosenCleaveBody();

  void onNetworkReplyFinished(QNetworkReply *reply);

private:
  ZFlyEmBody3dDoc *m_bodyDoc;
  uint64_t m_bodyId;
  int m_maxLevel;

  QWidget *m_widget;
  QSlider *m_levelSlider;
  QCheckBox *m_showCleavingCheckBox;
  QComboBox *m_cleaveIndexComboBox;
  QPushButton *m_selectBodyButton;
  QCheckBox *m_showSeedsOnlyCheckBox;
  QLabel *m_cleavingStatusLabel;
  QShortcut *m_shortcutToggle;
  QMenu *m_menu;
  QAction *m_showSeedsOnlyAction;
  QAction *m_toggleInBodyAction;
  std::map<QAction *, int> m_actionToComboBoxIndex;

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

  void cleave();

  void writeOutput(const ZDvidReader &reader, ZDvidWriter &writer,
                   const std::map<unsigned int, std::vector<uint64_t>> &cleaveIndexToMeshIds);
  void writeAuxiliaryOutput(const ZDvidReader &reader, ZDvidWriter &writer,
                            const std::map<unsigned int, std::vector<uint64_t>> &cleaveIndexToMeshIds);

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual void onCompleted() override;
};

#endif // TASKBODYCLEAVE_H
