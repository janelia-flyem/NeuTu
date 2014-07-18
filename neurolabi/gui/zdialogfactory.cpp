#include "zdialogfactory.h"
#include "zdviddialog.h"
#include "neutubeconfig.h"
#include "zstring.h"

ZDialogFactory::ZDialogFactory()
{
}

ZDvidDialog* ZDialogFactory::makeDvidDialog(QWidget *parent)
{
  ZDvidDialog *dlg = new ZDvidDialog(parent);
  dlg->loadConfig(ZString::fullPath(GET_APPLICATION_DIR,
                                    "json", "", "flyem_config.json"));

  return dlg;
}
