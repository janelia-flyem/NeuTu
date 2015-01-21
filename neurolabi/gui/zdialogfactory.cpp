#include "zdialogfactory.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "zwidgetfactory.h"
#include "zdviddialog.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "zparameterarray.h"

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
  /*
  dlg->loadConfig(ZString::fullPath(GET_APPLICATION_DIR,
                                    "json", "", "flyem_config.json"));
                                    */

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

QDialog* ZDialogFactory::makeParameterDialog(
    const ZParameterArray &parameterArray, QWidget *parent)
{
  QDialog *dlg = new QDialog(parent);
  QVBoxLayout *layout = new QVBoxLayout(dlg);
  foreach (QPointer<ZParameter> parameter, parameterArray.getData()) {
    if(parameter) {
      QHBoxLayout *wlayout = new QHBoxLayout(dlg);
      wlayout->addWidget(parameter->createNameLabel(dlg));
      wlayout->addWidget(parameter->createWidget(dlg));
      layout->addLayout(wlayout);
    }
  }

  return dlg;
}

bool ZDialogFactory::ask(
    const QString &title, const QString &msg, QWidget *parent)
{
  return QMessageBox::question(
        parent, title, msg, QMessageBox::No | QMessageBox::Yes) ==
      QMessageBox::Yes;
}

ZSpinBoxGroupDialog* ZDialogFactory::makeDownsampleDialog(QWidget *parent)
{
  ZSpinBoxGroupDialog *dlg = new ZSpinBoxGroupDialog(parent);
  dlg->setWindowTitle("Set Downsample Interval");
  dlg->addSpinBox("X", 0, 100, 0, 0);
  dlg->addSpinBox("Y", 0, 100, 0, 0);
  dlg->addSpinBox("Z", 0, 100, 0, 0);
  return dlg;
}
