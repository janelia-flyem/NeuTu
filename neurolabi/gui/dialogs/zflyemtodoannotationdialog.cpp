#include "zflyemtodoannotationdialog.h"
#include "ui_zflyemtodoannotationdialog.h"

#include "common/utilities.h"
#include "flyem/zflyemtodoitem.h"

ZFlyEmTodoAnnotationDialog::ZFlyEmTodoAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmTodoAnnotationDialog)
{
  ui->setupUi(this);
  initActionBox();
  initCheckedCombo();

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
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_DIAGNOSTIC);
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_SEGMENTATION_DIAGNOSTIC);
#ifdef _DEBUG_
  //For debugging only
  ui->actionComboBox->addItem(ZFlyEmToDoItem::ACTION_TIP_DETECTOR);
#endif

  connect(ui->actionComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateWidget()));
}

void ZFlyEmTodoAnnotationDialog::initCheckedCombo() {
    // this is a little clunky, but we expect this will change never or rarely
    ui->checkedComboBox->addItem(ZFlyEmToDoItem::TODO_STATE_CHECKED);
    ui->checkedComboBox->addItem(ZFlyEmToDoItem::TODO_STATE_CHECKED_WONT_DO);
    ui->checkedComboBox->addItem(ZFlyEmToDoItem::TODO_STATE_UNCHECKED);
}

void ZFlyEmTodoAnnotationDialog::updateWidget()
{
  if (ui->actionComboBox->currentText() == ZFlyEmToDoItem::ACTION_NO_SOMA) {
    ui->checkedComboBox->setCurrentText(ZFlyEmToDoItem::TODO_STATE_CHECKED);
  } else {
    ui->checkedComboBox->setCurrentText(QString::fromStdString(m_bufferCheckedString));
  }
}

void ZFlyEmTodoAnnotationDialog::init(const ZFlyEmToDoItem &item)
{
  ui->priorityComboBox->setCurrentText(item.getPriorityName().c_str());

  ui->checkedComboBox->setCurrentText(QString::fromStdString(item.getChecked()));
  m_bufferCheckedString = item.getChecked();

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
    item->setChecked(ui->checkedComboBox->currentText().toStdString());
    item->setPriority(getPriority());
    item->setComment(getComment().toStdString());
  }
}
