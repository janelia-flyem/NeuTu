#include "zflyemprojectmanager.h"

#include <QWidget>

#include "dialogs/flyembodymergeprojectdialog.h"
#include "dialogs/flyembodysplitprojectdialog.h"

ZFlyEmProjectManager::ZFlyEmProjectManager(QWidget *parent) :
  QObject(parent)
{
  m_mergeDlg = new FlyEmBodyMergeProjectDialog(parent);
  m_splitDlg = new FlyEmBodySplitProjectDialog(parent);

  connectSignalSlot();

  m_mergeDlg->enableMessageManager();
  m_splitDlg->enableMessageManager();
}

void ZFlyEmProjectManager::connectSignalSlot()
{
  connect(m_mergeDlg->getProject(), SIGNAL(splitSent(ZDvidTarget,uint64_t)),
          m_splitDlg, SLOT(startSplit(ZDvidTarget,uint64_t)));
}
