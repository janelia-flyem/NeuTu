#include "zcomboeditwidget.h"

#include <QHBoxLayout>

ZComboEditWidget::ZComboEditWidget(QWidget *parent) : QWidget(parent)
{
  m_comboBox = new QComboBox(this);
  m_textEdit = new QLineEdit(this);

  QLayout *layout = new QHBoxLayout;
  layout->addWidget(m_comboBox);
  layout->addWidget(m_textEdit);

  setLayout(layout);

  connectSignalSlot();
}

void ZComboEditWidget::setStringList(const QStringList &stringList)
{
  m_comboBox->clear();
  m_comboBox->addItem("---");
  m_comboBox->addItems(stringList);
}

QString ZComboEditWidget::getText() const
{
  return m_textEdit->text().trimmed();
}

void ZComboEditWidget::setCurrentIndex(int index)
{
  m_comboBox->setCurrentIndex(index);
}

void ZComboEditWidget::setStringList(const std::vector<std::string> &stringList)
{
  QStringList sl;
  for (std::vector<std::string>::const_iterator iter = stringList.begin();
       iter != stringList.end(); ++iter) {
    sl.append((*iter).c_str());
  }

  setStringList(sl);
}

void ZComboEditWidget::connectSignalSlot()
{
  connect(m_comboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setEditText()));
  connect(m_textEdit, SIGNAL(textChanged(QString)), this, SLOT(resetComboBox()));
}

void ZComboEditWidget::setEditText()
{
  if (m_comboBox->currentIndex() > 0) {
    m_textEdit->blockSignals(true);
    m_textEdit->setText(m_comboBox->currentText());
    m_textEdit->blockSignals(false);
  }
}

void ZComboEditWidget::resetComboBox()
{
  m_comboBox->blockSignals(true);
  m_comboBox->setCurrentIndex(0);
  m_comboBox->blockSignals(false);
}
