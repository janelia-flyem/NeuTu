#include "flyembodysupplywidget.h"

#include <QHBoxLayout>
#include <QDebug>

#include "zstring.h"

FlyEmBodySupplyWidget::FlyEmBodySupplyWidget(QWidget *parent) : QWidget(parent)
{
  m_layout = new QHBoxLayout(this);
  setLayout(m_layout);

  m_bodyEdit = new QLineEdit(this);
  m_bodyEdit->setPlaceholderText("Input body list");
  m_layout->addWidget(m_bodyEdit);

  m_confirmButton = new QPushButton(this);
  m_layout->addWidget(m_confirmButton);
  updateConfirmButton(m_bodyEdit->text());

  connect(m_bodyEdit, &QLineEdit::textChanged, this,
          &FlyEmBodySupplyWidget::processBodyEditChange);
  connect(m_confirmButton, &QPushButton::pressed,
          this, &FlyEmBodySupplyWidget::confirm);
  connect(this, &FlyEmBodySupplyWidget::confirmed,
          this, &FlyEmBodySupplyWidget::postConfirm);
}

void FlyEmBodySupplyWidget::postConfirm(const QList<uint64_t> &/*bodyStr*/)
{
  if (m_clearAfterConfirm) {
    clearEdit();
  }
}

QList<uint64_t> FlyEmBodySupplyWidget::parseBodyList(const QString &bodyStr)
{
  if (bodyStr == m_bodyParsingCache.first) {
    return m_bodyParsingCache.second;
  }

  std::vector<uint64_t> bodyArray =
      ZString(bodyStr.toStdString()).toUint64Array();
  QList<uint64_t> bodyList;
  for (uint64_t body : bodyArray) {
    bodyList.append(body);
  }
  m_bodyParsingCache.first = bodyStr;
  m_bodyParsingCache.second = bodyList;

  return bodyList;
}

void FlyEmBodySupplyWidget::updateConfirmButton()
{
  updateConfirmButton(m_bodyEdit->text());
}

void FlyEmBodySupplyWidget::updateConfirmButton(const QString &bodyStr)
{
  QList<uint64_t> bodyList = parseBodyList(bodyStr);
  m_confirmButton->setText(QString("%1 (%2)").arg(m_confirmTitle).arg(bodyList.size()));
  m_confirmButton->setDisabled(bodyList.isEmpty());
}

void FlyEmBodySupplyWidget::processBodyEditChange(const QString &bodyStr)
{
  updateConfirmButton(bodyStr);
}

void FlyEmBodySupplyWidget::clearEdit()
{
  m_bodyEdit->clear();
}

void FlyEmBodySupplyWidget::setConfirmTitle(const QString &title)
{
  m_confirmTitle = title;
  updateConfirmButton();
}

void FlyEmBodySupplyWidget::confirm()
{
#ifdef _DEBUG_
  qDebug() << "FlyEmBodySupplyWidget::confirm"
           << parseBodyList(m_bodyEdit->text());
#endif
  emit confirmed(parseBodyList(m_bodyEdit->text()));
}
