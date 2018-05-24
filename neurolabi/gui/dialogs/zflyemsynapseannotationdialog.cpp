#include "zflyemsynapseannotationdialog.h"

#include <sstream>
#include <iostream>

#include <QPainter>

#include "ui_zflyemsynapseannotationdialog.h"
#include "tz_math.h"
#include "zjsonobject.h"

ZFlyEmSynapseAnnotationDialog::ZFlyEmSynapseAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSynapseAnnotationDialog)
{
  ui->setupUi(this);

  m_confidence = 1.0;
  connect(ui->annotComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateAnnotationWidget()));
}

ZFlyEmSynapseAnnotationDialog::~ZFlyEmSynapseAnnotationDialog()
{
  delete ui;
}

void ZFlyEmSynapseAnnotationDialog::setOption(ZDvidAnnotation::EKind kind)
{
  ui->annotComboBox->clear();
  ui->annotComboBox->addItem("Custom");
  if (kind == ZDvidAnnotation::KIND_PRE_SYN) {
    ui->annotComboBox->addItem("Multi");
    ui->annotComboBox->addItem("Convergent");
  }
  ui->annotComboBox->addItem("Flagged");
}

void ZFlyEmSynapseAnnotationDialog::updateAnnotationWidget()
{
  ui->annotLineEdit->setVisible(ui->annotComboBox->currentIndex() == 0);
}

void ZFlyEmSynapseAnnotationDialog::setConfidence(double c)
{
  m_confidence = c;

  if (c > 1.0) {
    c = 1.0;
  }

  if (c < 0.0) {
    c = 0.0;
  }

  ui->confComboBox->setCurrentIndex(getConfidenceIndex(c));
}

int ZFlyEmSynapseAnnotationDialog::getConfidenceIndex(double c) const
{
  return iround((1.0 - c) / 0.5);
}

double ZFlyEmSynapseAnnotationDialog::getConfidence() const
{
  double conf = m_confidence;

  if (ui->confComboBox->currentIndex() != getConfidenceIndex(m_confidence)) {
    switch (ui->confComboBox->currentIndex()) {
    case 0:
      conf = 1.0;
      break;
    case 1:
      conf = 0.5;
      break;
    case 2:
      conf = 0.1;
      break;
    default:
      break;
    }
  }

  return conf;
}

void ZFlyEmSynapseAnnotationDialog::paintEvent(QPaintEvent *e)
{
  QDialog::paintEvent(e);

  if (ui->annotComboBox->currentIndex() == 0) {
    QPainter painter(this);
    QPen pen;
    pen.setWidth(2.0);
    pen.setColor(Qt::darkGray);
    painter.setPen(pen);

    QPoint start = ui->annotComboBox->frameGeometry().center();
    start.setY(start.y() + ui->annotComboBox->size().height() / 3);
    QPoint end = QPoint(start.x(), ui->annotLineEdit->frameGeometry().top());

    painter.drawLine(start, end);
  }
}

void ZFlyEmSynapseAnnotationDialog::setAnnotation(const QString &annotation)
{
  for (int i = 0; i < ui->annotComboBox->count(); ++i) {
    if (annotation == ui->annotComboBox->itemText(i)) {
      ui->annotComboBox->setCurrentIndex(i);

      return;
    }
  }

  ui->annotComboBox->setCurrentIndex(0);
  ui->annotLineEdit->setText(annotation);
}

QString ZFlyEmSynapseAnnotationDialog::getAnnotation() const
{
  if (ui->annotComboBox->currentIndex() == 0) {
    return ui->annotLineEdit->text().trimmed();
  }

  return ui->annotComboBox->currentText();
}

ZJsonObject ZFlyEmSynapseAnnotationDialog::getPropJson() const
{
  ZJsonObject propJson;
  std::ostringstream stream;
  stream << getConfidence();
  propJson.setEntry("conf", stream.str());

  QString annotation = getAnnotation();
  propJson.setEntry("annotation", annotation.toStdString());

#ifdef _DEBUG_
  std::cout << propJson.dumpString(2) << std::endl;
#endif

  return propJson;
}
