#include "flyemdialogfactory.h"

//#include "../zflyembodyannotationdialog.h"
#include "../zflyemproofdoc.h"
#include "../zflyemmisc.h"
#include "flyembodyannotationdialog.h"
#include "zgenericbodyannotationdialog.h"
#include "flyembatchbodyannotationdialog.h"

FlyEmDialogFactory::FlyEmDialogFactory()
{
}

/*
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
*/

FlyEmBodyAnnotationDialog* FlyEmDialogFactory::MakeBodyAnnotationDialog(
    ZFlyEmProofDoc *doc, QWidget *parent)
{
  FlyEmBodyAnnotationDialog *dlg =
      new FlyEmBodyAnnotationDialog(doc->isAdmin(), parent);
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

ZGenericBodyAnnotationDialog* FlyEmDialogFactory::MakeBodyAnnotaitonDialog(
    ZFlyEmProofDoc *doc, ZJsonObject config, QWidget *parent)
{
  ZGenericBodyAnnotationDialog *dlg = new ZGenericBodyAnnotationDialog(parent);
  dlg->setAdmin(doc->isAdmin());

  dlg->configure(config);

  QList<QString> statusList = doc->getBodyStatusList();

  if (!statusList.empty()) {
    dlg->setDefaultStatusList(statusList);
  } else {
    dlg->setDefaultStatusList(flyem::GetDefaultBodyStatus());
  }

  foreach (const QString &status, doc->getAdminBodyStatusList()) {
    dlg->addAdminStatus(status);
  }

  return dlg;
}

FlyEmBatchBodyAnnotationDialog* FlyEmDialogFactory::MakeBatchAnnotationDialog(
      ZFlyEmProofDoc *doc, ZJsonObject config, QWidget *parent)
{
  FlyEmBatchBodyAnnotationDialog *dlg = new FlyEmBatchBodyAnnotationDialog(parent);
  dlg->setAdmin(doc->isAdmin());

  dlg->configure(config);

  QList<QString> statusList = doc->getBodyStatusList();

  if (!statusList.empty()) {
    dlg->setDefaultStatusList(statusList);
  } else {
    dlg->setDefaultStatusList(flyem::GetDefaultBodyStatus());
  }
  foreach (const QString &status, doc->getAdminBodyStatusList()) {
    dlg->addAdminStatus(status);
  }

  return dlg;
}
