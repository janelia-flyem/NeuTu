#include "bcadjustdialog.h"
#include "ui_bcadjustdialog.h"

BcAdjustDialog::BcAdjustDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::BcAdjustDialog)
{
  m_ui->setupUi(this);

  init();
  connectSignalSlot();

  setLayout(m_ui->overallLayout);
}

BcAdjustDialog::~BcAdjustDialog()
{
  delete m_ui;
}

void BcAdjustDialog::init()
{
  m_lowerValueScrollBar.append(m_ui->lowerScrollBar);
  m_lowerValueScrollBar.append(m_ui->lowerScrollBar_2);
  m_lowerValueScrollBar.append(m_ui->lowerScrollBar_3);
  m_lowerValueScrollBar.append(m_ui->lowerScrollBar_4);

  m_upperValueScrollBar.append(m_ui->upperScrollBar);
  m_upperValueScrollBar.append(m_ui->upperScrollBar_2);
  m_upperValueScrollBar.append(m_ui->upperScrollBar_3);
  m_upperValueScrollBar.append(m_ui->upperScrollBar_4);

  m_lowerValueLabel.append(m_ui->labelLowerValue);
  m_lowerValueLabel.append(m_ui->labelLowerValue_2);
  m_lowerValueLabel.append(m_ui->labelLowerValue_3);
  m_lowerValueLabel.append(m_ui->labelLowerValue_4);

  m_upperValueLabel.append(m_ui->labelUpperValue);
  m_upperValueLabel.append(m_ui->labelUpperValue_2);
  m_upperValueLabel.append(m_ui->labelUpperValue_3);
  m_upperValueLabel.append(m_ui->labelUpperValue_4);

  for (int i = 0; i < m_lowerValueScrollBar.size(); ++i) {
    m_lowerValueScrollBar[i]->setRange(0, 0);
    m_upperValueScrollBar[i]->setRange(0, 0);

    m_lowerValueLabel[i]->setFixedSize(m_lowerValueLabel[i]->sizeHint());
    m_upperValueLabel[i]->setFixedSize(m_upperValueLabel[i]->sizeHint());
  }
}

void BcAdjustDialog::connectSignalSlot()
{
  connect(m_ui->lowerScrollBar, SIGNAL(valueChanged(int)),
          this, SLOT(setLowerValueLabel(int)));
  connect(m_ui->upperScrollBar, SIGNAL(valueChanged(int)),
          this, SLOT(setUpperValueLabel(int)));
  connect(m_ui->lowerScrollBar_2, SIGNAL(valueChanged(int)),
          this, SLOT(setLowerValueLabel_2(int)));
  connect(m_ui->upperScrollBar_2, SIGNAL(valueChanged(int)),
          this, SLOT(setUpperValueLabel_2(int)));
  connect(m_ui->lowerScrollBar_3, SIGNAL(valueChanged(int)),
          this, SLOT(setLowerValueLabel_3(int)));
  connect(m_ui->upperScrollBar_3, SIGNAL(valueChanged(int)),
          this, SLOT(setUpperValueLabel_3(int)));
  connect(m_ui->lowerScrollBar_4, SIGNAL(valueChanged(int)),
          this, SLOT(setLowerValueLabel_4(int)));
  connect(m_ui->upperScrollBar_4, SIGNAL(valueChanged(int)),
          this, SLOT(setUpperValueLabel_4(int)));
}

void BcAdjustDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void BcAdjustDialog::setLowerValueLabel(int value)
{
  if (value > m_ui->upperScrollBar->value()) {
    value = m_ui->upperScrollBar->value();
    m_ui->lowerScrollBar->setValue(value);
  }
  m_ui->labelLowerValue->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setUpperValueLabel(int value)
{
  if (value < m_ui->lowerScrollBar->value()) {
    value = m_ui->lowerScrollBar->value();
    m_ui->upperScrollBar->setValue(value);
  }
  m_ui->labelUpperValue->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setLowerValueLabel_2(int value)
{
  if (value > m_ui->upperScrollBar_2->value()) {
    value = m_ui->upperScrollBar_2->value();
    m_ui->lowerScrollBar_2->setValue(value);
  }
  m_ui->labelLowerValue_2->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setUpperValueLabel_2(int value)
{
  if (value < m_ui->lowerScrollBar_2->value()) {
    value = m_ui->lowerScrollBar_2->value();
    m_ui->upperScrollBar_2->setValue(value);
  }
  m_ui->labelUpperValue_2->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setLowerValueLabel_3(int value)
{
  if (value > m_ui->upperScrollBar_3->value()) {
    value = m_ui->upperScrollBar_3->value();
    m_ui->lowerScrollBar_3->setValue(value);
  }
  m_ui->labelLowerValue_3->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setUpperValueLabel_3(int value)
{
  if (value < m_ui->lowerScrollBar_3->value()) {
    value = m_ui->lowerScrollBar_3->value();
    m_ui->upperScrollBar_3->setValue(value);
  }
  m_ui->labelUpperValue_3->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setLowerValueLabel_4(int value)
{
  if (value > m_ui->upperScrollBar_4->value()) {
    value = m_ui->upperScrollBar_4->value();
    m_ui->lowerScrollBar_4->setValue(value);
  }
  m_ui->labelLowerValue_4->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setUpperValueLabel_4(int value)
{
  if (value < m_ui->lowerScrollBar_4->value()) {
    value = m_ui->lowerScrollBar_4->value();
    m_ui->upperScrollBar_4->setValue(value);
  }
  m_ui->labelUpperValue_4->setText(QString("%1").arg(value));
}

void BcAdjustDialog::setValue(int lower, int upper, int c)
{
  if (c >= 0 && c < m_lowerValueScrollBar.size()) {
    disconnect(m_lowerValueScrollBar[c], SIGNAL(valueChanged(int)),
               this, SIGNAL(valueChanged()));
    m_lowerValueScrollBar[c]->setValue(lower);
    connect(m_lowerValueScrollBar[c], SIGNAL(valueChanged(int)),
            this, SIGNAL(valueChanged()));

    disconnect(m_upperValueScrollBar[c], SIGNAL(valueChanged(int)),
               this, SIGNAL(valueChanged()));
    m_upperValueScrollBar[c]->setValue(upper);
    connect(m_upperValueScrollBar[c], SIGNAL(valueChanged(int)),
            this, SIGNAL(valueChanged()));
  }
}

// minimum neccessary range [0,255], otherwise uint8 image cannot go back to original
// state after ajusting
void BcAdjustDialog::setRange(int min, int max, int c)
{
  int actMin = std::min(0, min);
  int actMax = std::max(255, max);
  if (c==0) {
    m_ui->lowerScrollBar->setRange(actMin, actMax);
    m_ui->upperScrollBar->setRange(actMin, actMax);
    m_ui->labelMinmax->setText(QString("Grey level range: %1 ~ %2").
                               arg(min).arg(max));
  } else if (c==1) {
    m_ui->lowerScrollBar_2->setRange(actMin, actMax);
    m_ui->upperScrollBar_2->setRange(actMin, actMax);
    m_ui->labelMinmax_2->setText(QString("Grey level range: %1 ~ %2").
                               arg(min).arg(max));
  } else if (c==2) {
    m_ui->lowerScrollBar_3->setRange(actMin, actMax);
    m_ui->upperScrollBar_3->setRange(actMin, actMax);
    m_ui->labelMinmax_3->setText(QString("Grey level range: %1 ~ %2").
                               arg(min).arg(max));
  } else if (c==3) {
    m_ui->lowerScrollBar_4->setRange(actMin, actMax);
    m_ui->upperScrollBar_4->setRange(actMin, actMax);
    m_ui->labelMinmax_4->setText(QString("Grey level range: %1 ~ %2").
                               arg(min).arg(max));
  }
}

// if low == high, we will map [low-1, low] to [0, 255] to make sure
// pixel < low be zero and pixel >= low be 255

double BcAdjustDialog::getGreyScale(int c)
{
  if (c==0) {
    if ((m_ui->upperScrollBar->value() == m_ui->upperScrollBar->minimum()) &&
        (m_ui->lowerScrollBar->value() == m_ui->lowerScrollBar->minimum()) &&
        (m_ui->upperScrollBar->value() == m_ui->lowerScrollBar->value())) {
      return 0.0;
    }

    if (m_ui->upperScrollBar->value() == m_ui->lowerScrollBar->value())
      return 255.0;

    return 255.0 / (m_ui->upperScrollBar->value() -
                    m_ui->lowerScrollBar->value());
  } else if (c==1) {
    if ((m_ui->upperScrollBar_2->value() == m_ui->upperScrollBar_2->minimum()) &&
        (m_ui->lowerScrollBar_2->value() == m_ui->lowerScrollBar_2->minimum()) &&
        (m_ui->upperScrollBar_2->value() == m_ui->lowerScrollBar_2->value())) {
      return 0.0;
    }

    if (m_ui->upperScrollBar_2->value() == m_ui->lowerScrollBar_2->value())
      return 255.0;

    return 255.0 / (m_ui->upperScrollBar_2->value() -
                    m_ui->lowerScrollBar_2->value());
  } else if (c==2) {
    if ((m_ui->upperScrollBar_3->value() == m_ui->upperScrollBar_3->minimum()) &&
        (m_ui->lowerScrollBar_3->value() == m_ui->lowerScrollBar_3->minimum()) &&
        (m_ui->upperScrollBar_3->value() == m_ui->lowerScrollBar_3->value())) {
      return 0.0;
    }

    if (m_ui->upperScrollBar_3->value() == m_ui->lowerScrollBar_3->value())
      return 255.0;

    return 255.0 / (m_ui->upperScrollBar_3->value() -
                    m_ui->lowerScrollBar_3->value());
  } else {
    if ((m_ui->upperScrollBar_4->value() == m_ui->upperScrollBar_4->minimum()) &&
        (m_ui->lowerScrollBar_4->value() == m_ui->lowerScrollBar_4->minimum()) &&
        (m_ui->upperScrollBar_4->value() == m_ui->lowerScrollBar_4->value())) {
      return 0.0;
    }

    if (m_ui->upperScrollBar_4->value() == m_ui->lowerScrollBar_4->value())
      return 255.0;

    return 255.0 / (m_ui->upperScrollBar_4->value() -
                    m_ui->lowerScrollBar_4->value());
  }
}

double BcAdjustDialog::getGreyOffset(int c)
{
  if (c==0) {
    if ((m_ui->upperScrollBar->value() == m_ui->upperScrollBar->minimum()) &&
        (m_ui->lowerScrollBar->value() == m_ui->lowerScrollBar->minimum()) &&
        (m_ui->upperScrollBar->value() == m_ui->lowerScrollBar->value())) {
      return 255.0;
    }

    if (m_ui->upperScrollBar->value() == m_ui->lowerScrollBar->value())
      return -getGreyScale(c) * (m_ui->lowerScrollBar->value() - 1);

    return -getGreyScale(c) * m_ui->lowerScrollBar->value();
  } else if (c==1) {
    if ((m_ui->upperScrollBar_2->value() == m_ui->upperScrollBar_2->minimum()) &&
        (m_ui->lowerScrollBar_2->value() == m_ui->lowerScrollBar_2->minimum()) &&
        (m_ui->upperScrollBar_2->value() == m_ui->lowerScrollBar_2->value())) {
      return 255.0;
    }

    if (m_ui->upperScrollBar_2->value() == m_ui->lowerScrollBar_2->value())
      return -getGreyScale(c) * (m_ui->lowerScrollBar_2->value() - 1);

    return -getGreyScale(c) * m_ui->lowerScrollBar_2->value();
  } else if (c==2) {
    if ((m_ui->upperScrollBar_3->value() == m_ui->upperScrollBar_3->minimum()) &&
        (m_ui->lowerScrollBar_3->value() == m_ui->lowerScrollBar_3->minimum()) &&
        (m_ui->upperScrollBar_3->value() == m_ui->lowerScrollBar_3->value())) {
      return 255.0;
    }

    if (m_ui->upperScrollBar_3->value() == m_ui->lowerScrollBar_3->value())
      return -getGreyScale(c) * (m_ui->lowerScrollBar_3->value() - 1);

    return -getGreyScale(c) * m_ui->lowerScrollBar_3->value();
  } else {
    if ((m_ui->upperScrollBar_4->value() == m_ui->upperScrollBar_4->minimum()) &&
        (m_ui->lowerScrollBar_4->value() == m_ui->lowerScrollBar_4->minimum()) &&
        (m_ui->upperScrollBar_4->value() == m_ui->lowerScrollBar_4->value())) {
      return 255.0;
    }

    if (m_ui->upperScrollBar_4->value() == m_ui->lowerScrollBar_4->value())
      return -getGreyScale(c) * (m_ui->lowerScrollBar_4->value() - 1);

    return -getGreyScale(c) * m_ui->lowerScrollBar_4->value();
  }
}

void BcAdjustDialog::setNumOfChannel(int c)
{
  if (c < 4) {
    m_ui->ch4_label->setEnabled(false);
    m_ui->ch4_lower_label->setEnabled(false);
    m_ui->ch4_upper_label->setEnabled(false);
    m_ui->labelLowerValue_4->setEnabled(false);
    m_ui->labelMinmax_4->setEnabled(false);
    m_ui->labelUpperValue_4->setEnabled(false);
    m_ui->lowerScrollBar_4->setEnabled(false);
    m_ui->upperScrollBar_4->setEnabled(false);
  } else {
    m_ui->ch4_label->setEnabled(true);
    m_ui->ch4_lower_label->setEnabled(true);
    m_ui->ch4_upper_label->setEnabled(true);
    m_ui->labelLowerValue_4->setEnabled(true);
    m_ui->labelMinmax_4->setEnabled(true);
    m_ui->labelUpperValue_4->setEnabled(true);
    m_ui->lowerScrollBar_4->setEnabled(true);
    m_ui->upperScrollBar_4->setEnabled(true);
  }
  if (c < 3) {
    m_ui->ch3_label->setEnabled(false);
    m_ui->ch3_lower_label->setEnabled(false);
    m_ui->ch3_upper_label->setEnabled(false);
    m_ui->labelLowerValue_3->setEnabled(false);
    m_ui->labelMinmax_3->setEnabled(false);
    m_ui->labelUpperValue_3->setEnabled(false);
    m_ui->lowerScrollBar_3->setEnabled(false);
    m_ui->upperScrollBar_3->setEnabled(false);
  } else {
    m_ui->ch3_label->setEnabled(true);
    m_ui->ch3_lower_label->setEnabled(true);
    m_ui->ch3_upper_label->setEnabled(true);
    m_ui->labelLowerValue_3->setEnabled(true);
    m_ui->labelMinmax_3->setEnabled(true);
    m_ui->labelUpperValue_3->setEnabled(true);
    m_ui->lowerScrollBar_3->setEnabled(true);
    m_ui->upperScrollBar_3->setEnabled(true);
  }
  if (c < 2) {
    m_ui->ch2_label->setEnabled(false);
    m_ui->ch2_lower_label->setEnabled(false);
    m_ui->ch2_upper_label->setEnabled(false);
    m_ui->labelLowerValue_2->setEnabled(false);
    m_ui->labelMinmax_2->setEnabled(false);
    m_ui->labelUpperValue_2->setEnabled(false);
    m_ui->lowerScrollBar_2->setEnabled(false);
    m_ui->upperScrollBar_2->setEnabled(false);
  } else {
    m_ui->ch2_label->setEnabled(true);
    m_ui->ch2_lower_label->setEnabled(true);
    m_ui->ch2_upper_label->setEnabled(true);
    m_ui->labelLowerValue_2->setEnabled(true);
    m_ui->labelMinmax_2->setEnabled(true);
    m_ui->labelUpperValue_2->setEnabled(true);
    m_ui->lowerScrollBar_2->setEnabled(true);
    m_ui->upperScrollBar_2->setEnabled(true);
  }
  if (c < 1) {
    m_ui->ch1_label->setEnabled(false);
    m_ui->ch1_lower_label->setEnabled(false);
    m_ui->ch1_upper_label->setEnabled(false);
    m_ui->labelLowerValue->setEnabled(false);
    m_ui->labelMinmax->setEnabled(false);
    m_ui->labelUpperValue->setEnabled(false);
    m_ui->lowerScrollBar->setEnabled(false);
    m_ui->upperScrollBar->setEnabled(false);
  } else {
    m_ui->ch1_label->setEnabled(true);
    m_ui->ch1_lower_label->setEnabled(true);
    m_ui->ch1_upper_label->setEnabled(true);
    m_ui->labelLowerValue->setEnabled(true);
    m_ui->labelMinmax->setEnabled(true);
    m_ui->labelUpperValue->setEnabled(true);
    m_ui->lowerScrollBar->setEnabled(true);
    m_ui->upperScrollBar->setEnabled(true);
  }
}

void BcAdjustDialog::on_autoAdjustPushButton_clicked()
{
  emit autoAdjustTriggered();
}

void BcAdjustDialog::on_pushButton_clicked()
{
  m_ui->lowerScrollBar->setValue(m_ui->lowerScrollBar->minimum());
  m_ui->upperScrollBar->setValue(m_ui->upperScrollBar->maximum());
  m_ui->lowerScrollBar_2->setValue(m_ui->lowerScrollBar_2->minimum());
  m_ui->upperScrollBar_2->setValue(m_ui->upperScrollBar_2->maximum());
  m_ui->lowerScrollBar_3->setValue(m_ui->lowerScrollBar_3->minimum());
  m_ui->upperScrollBar_3->setValue(m_ui->upperScrollBar_3->maximum());
  m_ui->lowerScrollBar_4->setValue(m_ui->lowerScrollBar_4->minimum());
  m_ui->upperScrollBar_4->setValue(m_ui->upperScrollBar_4->maximum());
}
