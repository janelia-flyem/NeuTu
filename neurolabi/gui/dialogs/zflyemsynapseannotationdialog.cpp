#include "zflyemsynapseannotationdialog.h"

#include <sstream>
#include <iostream>

#include <QPainter>

#include "ui_zflyemsynapseannotationdialog.h"
#include "tz_math.h"
#include "zjsonobject.h"
#include "dvid/zdvidsynapse.h"

ZFlyEmSynapseAnnotationDialog::ZFlyEmSynapseAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSynapseAnnotationDialog)
{
  ui->setupUi(this);

//  m_confidence = -1.0;
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
  if (kind == ZDvidAnnotation::EKind::KIND_PRE_SYN) {
    ui->annotComboBox->addItem("Multi");
    ui->annotComboBox->addItem("Convergent");
  }
  ui->annotComboBox->addItem("Flagged");
}

void ZFlyEmSynapseAnnotationDialog::updateAnnotationWidget()
{
  ui->annotLineEdit->setVisible(ui->annotComboBox->currentIndex() == 0);
}

void ZFlyEmSynapseAnnotationDialog::setConfidence(std::string c)
{
  m_confidenceStr = c;

  /*
  if (c > 1.0) {
    c = 1.0;
  }

  if (c < 0.0) {
    c = 0.0;
  }
  */

  const int lastIndex = 4;
  int index = getConfidenceIndex(c);
  if (index == lastIndex) {
    if (ui->confComboBox->count() <= lastIndex) {
      ui->confComboBox->addItem(m_confidenceStr.c_str());
    } else {
      ui->confComboBox->setItemText(lastIndex, m_confidenceStr.c_str());
    }
  } else {
    if (ui->confComboBox->count() == lastIndex + 1) {
      ui->confComboBox->removeItem(lastIndex);
    }
  }

  ui->confComboBox->setCurrentIndex(index);
}

int ZFlyEmSynapseAnnotationDialog::getConfidenceIndex(
    const std::string &cstr) const
{
  if (!cstr.empty()) {
    double c = std::atof(cstr.c_str());
    if (std::fabs(c - 0.1) < 0.00001) {
      return 3;
    } else if (c == 1.0) {
      return 1;
    } else if (std::fabs(c - 0.5) < 0.00001) {
      return 2;
    } else {
      return 4;
    }
  }

  return 0;

  /*
  if (c < 0.0 || c > 1.0) {
    return 0;
  }

  return iround((1.0 - c) / 0.5) + 1;
  */
}

bool ZFlyEmSynapseAnnotationDialog::hasConfidence() const
{
  return !m_confidenceStr.empty();
}

std::string ZFlyEmSynapseAnnotationDialog::getConfidenceStr() const
{
  std::string conf = m_confidenceStr;

  if (ui->confComboBox->currentIndex() != getConfidenceIndex(m_confidenceStr)) {
    switch (ui->confComboBox->currentIndex()) {
    case 0:
      conf = "";
      break;
    case 1:
      conf = "1";
      break;
    case 2:
      conf = "0.5";
      break;
    case 3:
      conf = "0.1";
      break;
    default:
      break;
    }
  }

  return conf;
}

/*
double ZFlyEmSynapseAnnotationDialog::getConfidence() const
{
  double conf = m_confidence;

  if (ui->confComboBox->currentIndex() != getConfidenceIndex(m_confidence)) {
    switch (ui->confComboBox->currentIndex()) {
    case 1:
      conf = 1.0;
      break;
    case 2:
      conf = 0.5;
      break;
    case 3:
      conf = 0.1;
      break;
    default:
      break;
    }
  }

  return conf;
}
*/

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
  std::string conf = getConfidenceStr();
  if (!conf.empty()) {
    ZDvidSynapse::SetConfidenceProp(propJson, conf);
//    std::ostringstream stream;
//    stream << getConfidence();
//    propJson.setEntry("conf", stream.str());
  }

  QString annotation = getAnnotation();
  propJson.setEntry("annotation", annotation.toStdString());

#ifdef _DEBUG_
  std::cout << propJson.dumpString(2) << std::endl;
#endif

  return propJson;
}

void ZFlyEmSynapseAnnotationDialog::set(const ZDvidSynapse &synapse)
{
  setOption(synapse.getKind());
  setConfidence(synapse.getConfidenceStr());
  /*
  if (synapse.hasConfidenceProperty()) {
    setConfidence(synapse.getConfidence());
  } else {
    setConfidence(-1.0);
  }
  */
  setAnnotation(synapse.getAnnotation().c_str());
}
