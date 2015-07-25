#include "zflyemprojectmanager.h"

#include "dialogs/flyembodymergeprojectdialog.h"
#include "dialogs/flyembodysplitprojectdialog.h"

ZFlyEmProjectManager::ZFlyEmProjectManager(QObject *parent) :
  QObject(parent)
{
  m_mergeDlg = new FlyEmBodyMergeProjectDialog(dynamic_cast<QWidget*>(parent));
  m_splitDlg = new FlyEmBodySplitProjectDialog(dynamic_cast<QWidget*>(parent));

  connectSignalSlot();

  m_mergeDlg->enableMessageManager();
  m_splitDlg->enableMessageManager();
}

void ZFlyEmProjectManager::connectSignalSlot()
{
  connect(m_mergeDlg->getProject(), SIGNAL(splitSent(ZDvidTarget,uint64_t)),
          m_splitDlg, SLOT(startSplit(ZDvidTarget,uint64_t)));
}
