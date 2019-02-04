#include "flyemdialogfactory.h"

#include "../zflyembodyannotationdialog.h"
#include "../zflyemproofdoc.h"
#include "../zflyemmisc.h"

FlyEmDialogFactory::FlyEmDialogFactory()
{
}

ZFlyEmBodyAnnotationDialog* FlyEmDialogFactory::MakeBodyAnnotationDialog(
    ZFlyEmProofDoc *doc, QWidget *parent)
{
  ZFlyEmBodyAnnotationDialog *dlg = new ZFlyEmBodyAnnotationDialog(parent);
  QList<QString> statusList = doc->getBodyStatusList();

  if (!statusList.empty()) {
    dlg->setDefaultStatusList(statusList);
  } else {
    dlg->setDefaultStatusList(flyem::GetDefaultBodyStatus());
  }

  for (const QString &status : doc->getAdminBodyStatusList()) {
    dlg->addAdminStatus(status);
  }

  return dlg;
}
