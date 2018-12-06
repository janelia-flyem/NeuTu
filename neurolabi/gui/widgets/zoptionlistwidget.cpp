#include "zoptionlistwidget.h"
#include "ui_zoptionlistwidget.h"

ZOptionListWidget::ZOptionListWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZOptionListWidget)
{
  ui->setupUi(this);

  connect(ui->addPushButton, &QPushButton::clicked,
          this, &ZOptionListWidget::addSelectedOption);
}

ZOptionListWidget::~ZOptionListWidget()
{
  delete ui;
}

void ZOptionListWidget::setOptionList(const QStringList &strList)
{
  ui->optionComboBox->addItems(strList);
}

void ZOptionListWidget::setName(const QString &name)
{
  ui->nameLabel->setText(name);
}

QStringList ZOptionListWidget::getSelectedOptionList() const
{
  QString optionStr = ui->optionLineEdit->text();

  QStringList opts;
  for (QString &opt : optionStr.split(m_delimiter)) {
    opt = opt.trimmed();
    if (!opt.isEmpty()) {
      opts.append(opt);
    }
  }

  return opts;
}

void ZOptionListWidget::addSelectedOption()
{
  ui->optionLineEdit->setText(
        ui->optionLineEdit->text() +
        ui->optionComboBox->currentText() + m_delimiter + " ");
}
