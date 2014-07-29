#include "zdialogfactory.h"
#include <QVBoxLayout>
#include <QPushButton>
#include "zwidgetfactory.h"
#include "zdviddialog.h"
#include "neutubeconfig.h"
#include "zstring.h"


ZDialogFactory::ZDialogFactory(QWidget *parentWidget)
{
  m_parentWidget = parentWidget;
}

ZDialogFactory::~ZDialogFactory()
{

}

ZDvidDialog* ZDialogFactory::makeDvidDialog(QWidget *parent)
{
  ZDvidDialog *dlg = new ZDvidDialog(parent);
  dlg->loadConfig(ZString::fullPath(GET_APPLICATION_DIR,
                                    "json", "", "flyem_config.json"));

  return dlg;
}

DvidImageDialog* ZDialogFactory::makeDvidImageDialog(
    ZDvidDialog *dvidDlg, QWidget *parent)
{
  DvidImageDialog *dlg = new DvidImageDialog(parent);
  if (dvidDlg != NULL) {
    dlg->setDvidDialog(dvidDlg);
  }

  return dlg;
}

ZSpinBoxDialog* ZDialogFactory::makeSpinBoxDialog(QWidget *parent)
{
  ZSpinBoxDialog *dlg = new ZSpinBoxDialog(parent);
  return dlg;
}

QDialog* ZDialogFactory::makeTestDialog(QWidget *parent)
{
  QDialog *dlg = new QDialog(parent);
  QVBoxLayout *layout = new QVBoxLayout(dlg);
  dlg->setLayout(layout);

  layout->addWidget(ZWidgetFactory::makeLabledEditWidget(
                      "test", ZWidgetFactory::SPACER_NONE, dlg));


  QHBoxLayout *buttonLayout = new QHBoxLayout(dlg);
  buttonLayout->addSpacerItem(ZWidgetFactory::makeHSpacerItem());
  QPushButton *okButton = new QPushButton(dlg);
  buttonLayout->addWidget(okButton);
  okButton->setText("OK");

  layout->addLayout(buttonLayout);

  dlg->connect(okButton, SIGNAL(clicked()), dlg, SLOT(accept()));

  return dlg;
}
