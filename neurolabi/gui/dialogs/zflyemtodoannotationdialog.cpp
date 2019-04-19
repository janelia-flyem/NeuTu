#include "zflyemtodoannotationdialog.h"
#include "ui_zflyemtodoannotationdialog.h"

#include "flyem/zflyemtodoitem.h"

ZFlyEmTodoAnnotationDialog::ZFlyEmTodoAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmTodoAnnotationDialog)
{
  ui->setupUi(this);
  initActionBox();
}

ZFlyEmTodoAnnotationDialog::~ZFlyEmTodoAnnotationDialog()
{
  delete ui;
}

void ZFlyEmTodoAnnotationDialog::initActionBox()
{
  //Must follow the definition of neutube::EToDoAction
  ui->actionComboBox->addItem("normal");
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_MERGE);
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_SPLIT);
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_SUPERVOXEL_SPLIT);
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_IRRELEVANT);
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_TRACE_TO_SOMA);
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_NO_SOMA);

  connect(ui->actionComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateWidget()));
}

void ZFlyEmTodoAnnotationDialog::updateWidget()
{
  if (ui->actionComboBox->currentText() == ZFlyEmToDoItem::ACTION_NO_SOMA) {
    ui->checkedCheckBox->setChecked(true);
  } else {
    ui->checkedCheckBox->setChecked(m_bufferChecked);
  }
}

void ZFlyEmTodoAnnotationDialog::init(const ZFlyEmToDoItem &item)
{
  ui->priorityComboBox->setCurrentText(item.getPriorityName().c_str());
  /*
  switch (item.getPriority()) {
  case 0:
    ui->priorityComboBox->setCurrentIndex(3);
    break;
  case 1:
  case 2:
  case 3:
    ui->priorityComboBox->setCurrentIndex(0);
    break;
  case 4:
  case 5:
    ui->priorityComboBox->setCurrentIndex(1);
    break;
  default:
    ui->priorityComboBox->setCurrentIndex(2);
    break;
  }
  */

  ui->checkedCheckBox->setChecked(item.isChecked());
  m_bufferChecked = item.isChecked();

  int index = neutu::EnumValue(item.getAction());
  Q_ASSERT(index < ui->actionComboBox->maxCount());
  ui->actionComboBox->setCurrentIndex(index);
  ui->commentLineEdit->setText(item.getComment().c_str());
}

int ZFlyEmTodoAnnotationDialog::getPriority() const
{
  if (ui->priorityComboBox->currentText() == "Low") {
    return 6;
  } else if (ui->priorityComboBox->currentText() == "High") {
    return 2;
  } else if (ui->priorityComboBox->currentText() == "Medium") {
    return 4;
  }

  return 0;
}

QString ZFlyEmTodoAnnotationDialog::getComment() const
{
  return ui->commentLineEdit->text();
}

void ZFlyEmTodoAnnotationDialog::annotate(ZFlyEmToDoItem *item)
{
  if (item != NULL) {
    item->setAction(ui->actionComboBox->currentText().toStdString());
    item->setChecked(ui->checkedCheckBox->isChecked());
    item->setPriority(getPriority());
    item->setComment(getComment().toStdString());
  }
}
