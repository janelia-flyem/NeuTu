#include "zflyemsplituploadoptiondialog.h"

#include <QStatusBar>
#include <QStatusTipEvent>

#include "ui_zflyemsplituploadoptiondialog.h"

#include "logging/zqslog.h"
#include "zwidgetfactory.h"
#include "flyem/zflyembodyannotation.h"
#include "flyem/zflyemmisc.h"
#include "flyem/flyemdatareader.h"

ZFlyEmSplitUploadOptionDialog::ZFlyEmSplitUploadOptionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSplitUploadOptionDialog)
{
  ui->setupUi(this);

  flyem::PrepareBodyStatus(ui->statusComboBox);
  if (ui->statusComboBox->count() > 1) {
    ui->statusComboBox->setCurrentIndex(1);
  }
//  m_statusBar = new QStatusBar(this);
//  ui->statusBarLayout->addWidget(m_statusBar);
}

bool ZFlyEmSplitUploadOptionDialog::event(QEvent *e)
{
  if(e->type()==QEvent::StatusTip){
    QStatusTipEvent *ev = (QStatusTipEvent*)e;
//    m_statusBar->showMessage(ev->tip());
    if (ev->tip().isEmpty()) {
      ui->hintLabel->setText(ZWidgetFactory::MakeHintString("Hint"));
    } else {
      ui->hintLabel->setText(ZWidgetFactory::MakeHintString(ev->tip()));
    }
    return true;
  }
  return QDialog::event(e);
}

ZFlyEmSplitUploadOptionDialog::~ZFlyEmSplitUploadOptionDialog()
{
  delete ui;
}

void ZFlyEmSplitUploadOptionDialog::setDvidTarget(const ZDvidTarget &target)
{
  LINFO() << "Setting dvid env in ZFlyEmSplitUploadOptionDialog";
  m_dvidReader.open(target);
}

ZFlyEmBodyAnnotation ZFlyEmSplitUploadOptionDialog::getAnnotation(
    uint64_t bodyId, uint64_t newBodyId) const
{
  ZFlyEmBodyAnnotation annot;
  if (passingAnnotation()) {
    if (m_dvidReader.isReady()) {
      annot = FlyEmDataReader::ReadBodyAnnotation(m_dvidReader, bodyId);
    }
  }

  if (newComment()) {
    annot.setComment(getComment().toStdString());
  }

  if (ui->statusCheckBox->isChecked()) {
    annot.setStatus(getStatus().toStdString());
  }

  if (!annot.isEmpty()) {
    annot.setBodyId(newBodyId);
//    annot.setStatus("Not examined");
  }

  return annot;
}

bool ZFlyEmSplitUploadOptionDialog::passingAnnotation() const
{
  return ui->autoAnnotationCheckBox->isChecked();
}

void ZFlyEmSplitUploadOptionDialog::setPassingAnnotation(bool on)
{
  ui->autoAnnotationCheckBox->setChecked(on);
}

void ZFlyEmSplitUploadOptionDialog::setNewComment(bool on)
{
  ui->commentCheckBox->setChecked(on);
}

bool ZFlyEmSplitUploadOptionDialog::newComment() const
{
  return ui->commentCheckBox->isChecked();
}

QString ZFlyEmSplitUploadOptionDialog::getComment() const
{
  return ui->commentLineEdit->text();
}

QString ZFlyEmSplitUploadOptionDialog::getStatus() const
{
  QString status;
  if (ui->statusComboBox->currentIndex() > 0) {
    status = ui->statusComboBox->currentText();
  }

  return status;
}

void ZFlyEmSplitUploadOptionDialog::setComment(const QString &comment)
{
  ui->commentLineEdit->setText(comment);
}
