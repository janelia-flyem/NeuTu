#include "zflyemprojectmanager.h"

#include "flyembodymergeprojectdialog.h"
#include "flyembodysplitprojectdialog.h"

ZFlyEmProjectManager::ZFlyEmProjectManager(QObject *parent) :
  QObject(parent)
{
  m_mergeDlg = new FlyEmBodyMergeProjectDialog(dynamic_cast<QWidget*>(parent));
  m_splitDlg = new FlyEmBodySplitProjectDialog(dynamic_cast<QWidget*>(parent));

  connectSignalSlot();
}

void ZFlyEmProjectManager::connectSignalSlot()
{
  connect(m_mergeDlg->getProject(), SIGNAL(splitSent(ZDvidTarget,int)),
          m_splitDlg, SLOT(startSplit(ZDvidTarget,int)));
}
