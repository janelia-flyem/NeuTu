#ifndef ZFLYEMPROJECTMANAGER_H
#define ZFLYEMPROJECTMANAGER_H

#include <QObject>

class QWidget;
class FlyEmBodyMergeProjectDialog;
class FlyEmBodySplitProjectDialog;

class ZFlyEmProjectManager : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmProjectManager(QWidget *parent = 0);

  inline FlyEmBodyMergeProjectDialog* getMergeDialog() { return m_mergeDlg; }
  inline FlyEmBodySplitProjectDialog* getSplitDialog() { return m_splitDlg; }

signals:

public slots:

private:
  void connectSignalSlot();

private:
  FlyEmBodyMergeProjectDialog *m_mergeDlg;
  FlyEmBodySplitProjectDialog *m_splitDlg;
};

#endif // ZFLYEMPROJECTMANAGER_H
