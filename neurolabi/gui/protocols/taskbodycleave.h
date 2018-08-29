#ifndef TASKBODYCLEAVE_H
#define TASKBODYCLEAVE_H

#include "protocols/taskprotocoltask.h"
#include "zpoint.h"
#include <QObject>
#include <QVector>
#include <set>

class ZFlyEmBody3dDoc;
class ZMesh;
class QAction;
class QCheckBox;
class QComboBox;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QShortcut;

class TaskBodyCleave : public TaskProtocolTask
{
  Q_OBJECT
public:
  TaskBodyCleave(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);
  QString tasktype() const override;
  QString actionString() override;
  QString targetString() override;

  virtual void beforeNext() override;
  virtual void beforePrev() override;
  virtual void beforeDone() override;

  virtual QWidget *getTaskWidget() override;
  virtual QMenu *getTaskMenu() override;

  uint64_t getBodyId() const;

  ProtocolTaskConfig getTaskConfig() const override;

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
  void onToggleShowChosenCleaveBody();
  void onHideSelected();
  void onClearHidden();
  void onChooseCleaveMethod();

  void onNetworkReplyFinished(QNetworkReply *reply);

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
  QAction *m_toggleInBodyAction;
  QAction *m_toggleShowChosenCleaveBodyAction;
  std::map<QAction *, int> m_actionToComboBoxIndex;

  // The cleave index assignments created by the last cleaving operation (initially empty).
  std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndex;

  // The cleave index assignments specified by the user, to be used as seeds for the next
  // cleaving operation.
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndex;

  std::set<size_t> m_hiddenCleaveIndices;

  QString m_cleaveMethod;

  QNetworkAccessManager *m_networkManager;
  bool m_cleaveReplyPending = false;

  // The latest cleave server reply that was applied is saved for debugging purposes.
  QJsonObject m_cleaveReply;

  std::set<QString> m_warningTextToSuppress;

  class SetCleaveIndicesCommand;
  class CleaveCommand;

  std::size_t chosenCleaveIndex() const;

  std::set<uint64_t> m_hiddenIds;

  void buildTaskWidget();
  void updateColors();

  void bodiesForCleaveIndex(std::set<uint64_t>& result, std::size_t cleaveIndex,
                            bool ignoreSeedsOnly = false);

  void selectBodies(const std::set<uint64_t>& toSelect);

  void applyPerTaskSettings();
  void applyColorMode(bool showingCleaving);
  void enableCleavingUI(bool showingCleaving);

  void cleave();

  void updateVisibility();

  std::set<std::size_t> hiddenChanges(const std::map<uint64_t, std::size_t>& newMeshIdToCleaveIndex) const;
  void showHiddenChangeWarning(const std::set<std::size_t>& hiddenChangedIndices);

  bool showCleaveReplyWarnings(const QJsonObject& reply);
  bool showCleaveReplyOmittedMeshes(std::map<uint64_t, std::size_t> meshIdToCleaveIndex);
  void displayWarning(const QString& title, const QString& text,
                      const QString& details = "",
                      bool allowSuppression = false);

  virtual bool loadSpecific(QJsonObject json) override;
  virtual QJsonObject addToJson(QJsonObject json) override;
  virtual bool allowCompletion() override;
  virtual void onCompleted() override;
};

#endif // TASKBODYCLEAVE_H
