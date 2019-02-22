#include "flyemmvcdialogmanager.h"

#include "zdialogfactory.h"

FlyEmMvcDialogManager::FlyEmMvcDialogGroup(ZFlyEmProofMvc *parent) :
  m_parent(parent)
{
}


ZDvidTargetProviderDialog* FlyEmMvcDialogManager::getDvidDlg()
{
  if (!m_dvidDlg) {
    m_dvidDlg = ZDialogFactory::makeDvidDialog(m_parent);
  }

  return m_dvidDlg;
}

