#include "zcolormapeditor.h"

#include "zcolormap.h"
#include "QsLog.h"
#include <QColorDialog>
#include <QMouseEvent>
#include <QMenu>
#include <QInputDialog>
#include <QToolTip>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>

ZColorMapWidget::ZColorMapWidget(ZColorMapParameter* colorMap, QWidget* parent) :
  QWidget(parent), m_colorMap(colorMap), m_sliderWidth(14), m_sliderHeight(18), m_isDragging(false)
  , m_dragStartX(-1)
{
  setFocusPolicy(Qt::StrongFocus);

  setMinimumHeight(minimumHeight() + m_sliderHeight - 1);
  int l, t, r, b;
  getContentsMargins(&l, &t, &r, &b);
  int margin = 15;
  setContentsMargins(l + margin, t + margin, r + margin, b + m_sliderHeight - 1 + margin);
  connect(m_colorMap, &ZColorMapParameter::valueChanged, this, &ZColorMapWidget::updateIntensityScreenWidth);
  updateIntensityScreenWidth();
}

void ZColorMapWidget::editLColor(size_t index)
{
  if (!m_colorMap || !(index < m_colorMap->get().numKeys()))
    return;

  ZColorMapKey key = m_colorMap->get().key(index);
  QColor newColor = QColorDialog::getColor(key.qColorL());
  if (newColor.isValid()) {
    m_colorMap->get().setKeyColorL(index, newColor);
  }
}

void ZColorMapWidget::editRColor(size_t index)
{
  if (!m_colorMap || !(index < m_colorMap->get().numKeys()))
    return;

  ZColorMapKey key = m_colorMap->get().key(index);
  QColor newColor = QColorDialog::getColor(key.qColorR());
  if (newColor.isValid()) {
    m_colorMap->get().setKeyColorR(index, newColor);
  }
}

void ZColorMapWidget::mousePressEvent(QMouseEvent* e)
{
  if (e->button() != Qt::LeftButton)
    return;

  size_t index;
  m_isDragging = false;
  if (findkey(e->pos(), index) != ZColorMapWidget::NONE) {
    m_pressedIndex = index;
    m_pressedPosX = e->pos().x();
    if (!m_colorMap->get().isKeySelected(index)) {
      if (e->modifiers() & Qt::ControlModifier) {
        m_colorMap->get().setKeySelected(index, true);
      } else {
        for (size_t i = 1; i < m_colorMap->get().numKeys() - 1; ++i) {
          if (index == i)
            m_colorMap->get().setKeySelected(i, true);
          else
            m_colorMap->get().setKeySelected(i, false);
        }
      }
    }
    m_dragStartX = e->pos().x();
    m_isDragging = true;
  } else {
    if (!(e->modifiers() & Qt::ControlModifier))
      for (size_t i = 0; i < m_colorMap->get().numKeys(); ++i) {
        m_colorMap->get().setKeySelected(i, false);
      }
  }
}

void ZColorMapWidget::mouseMoveEvent(QMouseEvent* e)
{
  if (!m_isDragging)
    return;
  int dist = e->pos().x() - m_dragStartX;
  if (dist == 0)
    return;
  bool change = true;
  size_t edgeKeyIndex = m_colorMap->get().numKeys();
  if (dist < 0) {
    // need to check from index 0 since selected key might be inserted before first key
    // during move, but it is impossible to go after last key because the clamp below
    for (size_t i = 0; i < m_colorMap->get().numKeys() - 1; ++i) {
      //LOG(INFO) << m_colorMap->get().isKeySelected(i) << " " << m_colorMap->get().key(i).intensity() << " " << m_colorMap->get().domainMin();
      if (m_colorMap->get().isKeySelected(i)) {
        if (m_colorMap->get().key(i).intensity() == m_colorMap->get().domainMin())
          change = false;
        edgeKeyIndex = i;
        break;
      }
    }
  } else {
    for (int i = int(m_colorMap->get().numKeys()) - 2; i >= 0; --i) {
      //LOG(INFO) << m_colorMap->get().isKeySelected(i) << " " << m_colorMap->get().key(i).intensity() << " " << m_colorMap->get().domainMax();
      if (m_colorMap->get().isKeySelected(i)) {
        if (m_colorMap->get().key(i).intensity() == m_colorMap->get().domainMax())
          change = false;
        edgeKeyIndex = i;
        break;
      }
    }
  }
  //LOG(INFO) << edgeKeyIndex << " " << m_colorMap->get().numKeys();
  CHECK(edgeKeyIndex != m_colorMap->get().numKeys());

  if (change) {
    double intensityChange = dist / m_intensityScreenWidth;
    if (dist < 0) {
      intensityChange = std::max(intensityChange, m_colorMap->get().domainMin() -
                                                  m_colorMap->get().keyIntensity(edgeKeyIndex));
    } else {
      intensityChange = std::min(intensityChange, m_colorMap->get().domainMax() -
                                                  m_colorMap->get().keyIntensity(edgeKeyIndex));
    }
    std::vector<ZColorMapKey> newKeys;
    for (size_t i = 0; i < m_colorMap->get().numKeys() - 1; ++i) {
      if (m_colorMap->get().isKeySelected(i)) {
        ZColorMapKey newKey = m_colorMap->get().key(i);
        double newIntensity = newKey.intensity() + intensityChange;
        if (newIntensity < m_colorMap->get().domainMin())
          newIntensity = m_colorMap->get().domainMin();
        else if (newIntensity > m_colorMap->get().domainMax())
          newIntensity = m_colorMap->get().domainMax();
        newKey.setIntensity(newIntensity);
        newKeys.push_back(newKey);
      }
    }
    m_colorMap->get().removeSelectedKeys();
    for (size_t i = 0; i < newKeys.size(); ++i) {
      m_colorMap->get().addKey(newKeys[i], true);
    }
  }
  m_dragStartX = e->pos().x();
}

void ZColorMapWidget::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() != Qt::LeftButton)
    return;
  m_isDragging = false;
  m_colorMap->get().removeDuplicatedKeys();
  m_colorMap->get().setKeySelected(0, false);
  m_colorMap->get().setKeySelected(m_colorMap->get().numKeys() - 1, false);
}

void ZColorMapWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
  if (e->button() != Qt::LeftButton) {
    e->ignore();
    return;
  }
  size_t index;
  if (findkey(e->pos(), index, true) == ZColorMapWidget::LEFT) {
    editLColor(index);
    return;
  }
  if (findkey(e->pos(), index, true) == ZColorMapWidget::RIGHT) {
    editRColor(index);
    return;
  }
  if (contentsRect().contains(e->pos())) {
    double i = screenXPositionToIntensity(e->pos().x());
    m_colorMap->get().addKeyAtIntensity(i);
    m_colorMap->get().removeDuplicatedKeys();
  }
}

void ZColorMapWidget::contextMenuEvent(QContextMenuEvent* e)
{
  size_t index;
  if (findkey(e->pos(), index) != ZColorMapWidget::NONE) {
    QMenu contextMenu(this);
    contextMenu.setTitle(QString("Key %1").arg(index + 1));
    bool isSplit = m_colorMap->get().key(index).isSplit();
    QAction* mergeAction = nullptr;
    QAction* editLColorAction = nullptr;
    QAction* editRColorAction = nullptr;
    QAction* splitAction = nullptr;
    QAction* editIntensityAction = nullptr;
    QAction* removeAction = nullptr;
    if (isSplit) {
      editLColorAction = contextMenu.addAction("Change Left Color");
      editRColorAction = contextMenu.addAction("Change Right Color");
      editIntensityAction = contextMenu.addAction("Change Intensity");
      mergeAction = contextMenu.addAction("Merge Left and Right");
    } else {
      editLColorAction = contextMenu.addAction("Change Color");
      editIntensityAction = contextMenu.addAction("Change Intensity");
      splitAction = contextMenu.addAction("Split Key");
    }
    removeAction = contextMenu.addAction("Delete");
    QAction* action = contextMenu.exec(e->globalPos());
    if (action == editLColorAction) {
      editLColor(index);
    } else if (action == editIntensityAction) {
      bool ok;
      double newI = QInputDialog::getDouble(this, QString("Key %1").arg(index + 1), "Intensity:",
                                            m_colorMap->get().key(index).intensity(),
                                            m_colorMap->get().domainMin() + 0.001,
                                            m_colorMap->get().domainMax() - 0.001,
                                            3, &ok);
      if (ok)
        m_colorMap->get().setKeyIntensity(index, newI);
    } else if (action == removeAction) {
      m_colorMap->get().removeKey(index);
    }
    if (isSplit) {
      if (action == editRColorAction) {
        editRColor(index);
      } else if (action == mergeAction) {
        m_colorMap->get().setKeySplit(index, false);
      }
    } else {
      if (action == splitAction)
        m_colorMap->get().setKeySplit(index, true);
    }
  } else if (contentsRect().contains(e->pos())) {
    double intensity = screenXPositionToIntensity(e->pos().x());
    QMenu contextMenu(this);
    QAction* addKeyHereAction = contextMenu.addAction(
      QString("Add Key Here (%1)").arg(intensity));
    QAction* addKeyAtAction = contextMenu.addAction("Add Key At Intensity...");
    QAction* action = contextMenu.exec(e->globalPos());

    if (action == addKeyHereAction) {
      m_colorMap->get().addKeyAtIntensity(intensity);
    } else if (action == addKeyAtAction) {
      double newI = QInputDialog::getDouble(this, QString("Add Key At"), "Intensity:",
                                            intensity);
      m_colorMap->get().addKeyAtIntensity(newI);
    }
  }
}

void ZColorMapWidget::keyPressEvent(QKeyEvent* e)
{
  switch (e->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace: {
      m_colorMap->get().removeSelectedKeys();
    }
  }
}

bool ZColorMapWidget::event(QEvent* e)
{
  if (e->type() == QEvent::ToolTip) {
    size_t index;
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
    QRect tipRect;
    QString tipText;
    if (findkey(helpEvent->pos(), index, true) != ZColorMapWidget::NONE) {
      tipRect = sliderBounds(index);
      if (m_colorMap->get().key(index).isSplit())
        tipText = QString("Key %1\nIntensity: %2\nLeft Color: %3\nRight Color: %4").arg(index + 1).arg(
            m_colorMap->get().key(index).intensity())
          .arg(m_colorMap->get().key(index).qColorL().name()).arg(m_colorMap->get().key(index).qColorR().name());
      else
        tipText = QString("Key %1\nIntensity: %2\nColor: %3").arg(index + 1).arg(
            m_colorMap->get().key(index).intensity())
          .arg(m_colorMap->get().key(index).qColorL().name());
      QToolTip::showText(helpEvent->globalPos(), tipText, this, tipRect);
    } else if (contentsRect().contains(helpEvent->pos())) {
      tipRect.setCoords(helpEvent->pos().x(), contentsRect().top(), helpEvent->pos().x(), contentsRect().bottom());
      QColor col = m_colorMap->get().mappedQColor(screenXPositionToIntensity(helpEvent->pos().x()));
      tipText = QString("Color: %1").arg(col.name());
      QToolTip::showText(helpEvent->globalPos(), tipText, this, tipRect);
    } else
      QToolTip::hideText();
  }
  return QWidget::event(e);
}

void ZColorMapWidget::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
  //

  for (int x = contentsRect().left(); x <= contentsRect().right(); ++x) {
    painter.setPen(
      m_colorMap->get().fractionMappedQColor((x - contentsRect().left()) / (contentsRect().width() * 1.0)));
    painter.drawLine(x, contentsRect().top(), x, contentsRect().bottom());
  }

  painter.setRenderHint(QPainter::Antialiasing);

  QPen pen(Qt::black, 1.);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);
  QPolygon polygon;
  polygon << QPoint(-m_sliderWidth / 2, static_cast<int>(m_sliderHeight * 0.3))
          << QPoint(-m_sliderWidth / 2, m_sliderHeight)
          << QPoint(m_sliderWidth / 2, m_sliderHeight)
          << QPoint(m_sliderWidth / 2, static_cast<int>(m_sliderHeight * 0.3))
          << QPoint(0, 0);
  polygon.translate(0, contentsRect().bottom() - 1.5);
  double prevPos = -m_sliderWidth;
  for (size_t i = 0; i < m_colorMap->get().numKeys(); ++i) {
    ZColorMapKey key = m_colorMap->get().key(i);
    double x = intensityToScreenXPosition(m_colorMap->get().key(i).intensity());
    polygon.translate(x - polygon[4].x(), 0);
    double midPos = (prevPos + x) / 2;
    QRect cr(static_cast<int>(midPos), 0, width(), height());
    if (key.isSplit())
      cr.setRight(static_cast<int>(x));
    painter.setClipRect(cr);
    painter.setBrush(key.qColorL());
    if (m_colorMap->get().isKeySelected(i)) {
      pen.setWidth(2.);
      painter.setPen(pen);
    }
    painter.drawPolygon(polygon);
    if (m_colorMap->get().isKeySelected(i)) {
      pen.setWidth(1.);
      painter.setPen(pen);
    }

    prevPos = x + 1;
    if (key.isSplit()) {
      painter.setClipRect(QRect(static_cast<int>(x), 0, width() + m_sliderWidth, height()));
      painter.setBrush(key.qColorR());
      if (m_colorMap->get().isKeySelected(i)) {
        pen.setWidth(2.);
        painter.setPen(pen);
      }
      painter.drawPolygon(polygon);
      if (m_colorMap->get().isKeySelected(i)) {
        pen.setWidth(1.);
        painter.setPen(pen);
      }
    }
  }

}

void ZColorMapWidget::resizeEvent(QResizeEvent* /*event*/)
{
  updateIntensityScreenWidth();
}

QSize ZColorMapWidget::sizeHint() const
{
  return QSize(255 + 30, 50 + 30);
}

ZColorMapWidget::FindKeyResult ZColorMapWidget::findkey(const QPoint& pos, size_t& index, bool includeBoundKey) const
{
  if (pos.y() > contentsRect().bottom() + m_sliderHeight - 1 ||
      pos.y() < contentsRect().bottom() - 1)
    return ZColorMapWidget::NONE;
  double minDist = 1e5;
  bool hit = false;
  bool left = true;
  size_t minIndex = 0;
  for (size_t i = 1; i < m_colorMap->get().numKeys() - 1; ++i) {
    double dist = intensityToScreenXPosition(m_colorMap->get().key(i).intensity()) - pos.x();
    if (std::abs(dist) < m_sliderWidth / 2 && std::abs(dist) < minDist) {
      hit = true;
      minIndex = i;
      left = dist >= 0;
    }
  }
  if (includeBoundKey) {
    double dist = std::abs(intensityToScreenXPosition(m_colorMap->get().key(0).intensity()) - pos.x());
    if (dist < m_sliderWidth / 2 && dist < minDist) {
      hit = true;
      minIndex = 0;
      left = false;
    }
    dist = std::abs(
      intensityToScreenXPosition(m_colorMap->get().key(m_colorMap->get().numKeys() - 1).intensity()) - pos.x());
    if (dist < m_sliderWidth / 2 && dist < minDist) {
      hit = true;
      minIndex = m_colorMap->get().numKeys() - 1;
      left = true;
    }
  }
  if (hit) {
    index = minIndex;
    return left ? ZColorMapWidget::LEFT : ZColorMapWidget::RIGHT;
  }
  return ZColorMapWidget::NONE;
}

double ZColorMapWidget::intensityToScreenXPosition(double intensity) const
{
  return contentsRect().left() + (intensity - m_colorMap->get().domainMin()) * m_intensityScreenWidth;
}

double ZColorMapWidget::screenXPositionToIntensity(double x) const
{
  return m_colorMap->get().domainMin() + (x - contentsRect().left()) / m_intensityScreenWidth;
}

QRect ZColorMapWidget::sliderBounds(size_t index) const
{
  QRect result(-m_sliderWidth / 2, contentsRect().bottom() + m_sliderHeight - 1, m_sliderWidth, m_sliderHeight);
  result.translate(intensityToScreenXPosition(m_colorMap->get().key(index).intensity()), -m_sliderHeight);
  return result;
}

void ZColorMapWidget::updateIntensityScreenWidth()
{
  m_intensityScreenWidth = contentsRect().width() /
                           (m_colorMap->get().domainMax() - m_colorMap->get().domainMin());
  update();
}

ZColorMapEditor::ZColorMapEditor(ZColorMapParameter* colorMap, QWidget* parent)
  : QWidget(parent), m_colorMap(colorMap)
{
  createWidget();
  connect(m_colorMap, &ZColorMapParameter::valueChanged, this, &ZColorMapEditor::updateFromColorMap);
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0) && __cplusplus > 201103L
  connect(m_domainMinSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ZColorMapEditor::setDomainMin);
  connect(m_domainMaxSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ZColorMapEditor::setDomainMax);
#else
  connect(m_domainMinSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setDomainMin(double)));
  connect(m_domainMaxSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setDomainMax(double)));
#endif
  connect(m_fitDomainToDataButton, &QPushButton::clicked, this, &ZColorMapEditor::fitDomainToDataRange);
}

void ZColorMapEditor::updateFromColorMap()
{
  if (m_colorMap->get().hasDataRange()) {
    m_dataMinValueLabel->setText(QString("%1").arg(m_colorMap->get().dataMin()));
    m_dataMaxValueLabel->setText(QString("%1").arg(m_colorMap->get().dataMax()));
  } else {
    m_dataMinValueLabel->setText("none");
    m_dataMaxValueLabel->setText("none");
  }
  m_domainMinSpinBox->setValue(m_colorMap->get().domainMin());
  m_domainMaxSpinBox->setValue(m_colorMap->get().domainMax());
  if (m_colorMap->get().hasDataRange())
    m_fitDomainToDataButton->setEnabled(true);
  else
    m_fitDomainToDataButton->setEnabled(false);
}

void ZColorMapEditor::setDomainMin(double min)
{
  if (m_colorMap->get().isValidDomainMin(min)) {
    m_colorMap->get().setDomainMin(min, m_rescaleKeys->isChecked());
  } else {
    QMessageBox::critical(this, qApp->applicationName(), "invalid colormap range start");
    m_domainMinSpinBox->setValue(m_colorMap->get().domainMin());
  }
}

void ZColorMapEditor::setDomainMax(double max)
{
  if (m_colorMap->get().isValidDomainMax(max)) {
    m_colorMap->get().setDomainMax(max, m_rescaleKeys->isChecked());
  } else {
    QMessageBox::critical(this, qApp->applicationName(), "invalid colormap range end");
    m_domainMaxSpinBox->setValue(m_colorMap->get().domainMax());
  }
}

void ZColorMapEditor::fitDomainToDataRange()
{
  m_colorMap->get().setDomainMin(m_colorMap->get().dataMin(), m_rescaleKeys->isChecked());
  m_colorMap->get().setDomainMax(m_colorMap->get().dataMax(), m_rescaleKeys->isChecked());
}

void ZColorMapEditor::createWidget()
{
  auto vlo = new QVBoxLayout;
  auto hlo = new QHBoxLayout;
  m_colorMapWidget = new ZColorMapWidget(m_colorMap, this);
  vlo->addWidget(m_colorMapWidget);
  m_dataMinNameLabel = new QLabel("Data Min: ", this);
  m_dataMinNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_dataMaxNameLabel = new QLabel("Data Max: ", this);
  m_dataMaxNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  if (m_colorMap->get().hasDataRange()) {
    m_dataMinValueLabel = new QLabel(QString("%1").arg(m_colorMap->get().dataMin()), this);
    m_dataMaxValueLabel = new QLabel(QString("%1").arg(m_colorMap->get().dataMax()), this);
  } else {
    m_dataMinValueLabel = new QLabel("none", this);
    m_dataMaxValueLabel = new QLabel("none", this);
  }
  m_dataMinValueLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_dataMaxValueLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  hlo->addWidget(m_dataMinNameLabel);
  hlo->addWidget(m_dataMinValueLabel);
  hlo->addStretch();
  hlo->addWidget(m_dataMaxNameLabel);
  hlo->addWidget(m_dataMaxValueLabel);
  hlo->addStretch();
  vlo->addLayout(hlo);
  hlo = new QHBoxLayout;
  m_domainMinNameLabel = new QLabel("Colormap Start: ", this);
  m_domainMaxNameLabel = new QLabel("Colormap End: ", this);
  m_domainMinNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_domainMaxNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_domainMinSpinBox = new QDoubleSpinBox(this);
  m_domainMaxSpinBox = new QDoubleSpinBox(this);
  m_domainMinSpinBox->setRange(-1e10, 1e10);
  m_domainMinSpinBox->setKeyboardTracking(false);
  m_domainMinSpinBox->setDecimals(4);
  m_domainMaxSpinBox->setRange(-1e10, 1e10);
  m_domainMaxSpinBox->setDecimals(4);
  m_domainMaxSpinBox->setKeyboardTracking(false);
  m_domainMinSpinBox->setValue(m_colorMap->get().domainMin());
  m_domainMaxSpinBox->setValue(m_colorMap->get().domainMax());
  m_fitDomainToDataButton = new QPushButton("Fit to Data", this);
  m_fitDomainToDataButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  if (m_colorMap->get().hasDataRange())
    m_fitDomainToDataButton->setEnabled(true);
  else
    m_fitDomainToDataButton->setEnabled(false);
  m_rescaleKeys = new QCheckBox("Rescale Keys", this);
  m_rescaleKeys->setToolTip(
    "If set, reserve all keys (by rescaling) while changing the domain. Otherwise out of domain keys will be deleted.");
  m_rescaleKeys->setChecked(false);
  hlo->addWidget(m_domainMinNameLabel);
  hlo->addWidget(m_domainMinSpinBox);
  hlo->addWidget(m_domainMaxNameLabel);
  hlo->addWidget(m_domainMaxSpinBox);
  hlo->addWidget(m_fitDomainToDataButton);
  hlo->addWidget(m_rescaleKeys);
  vlo->addLayout(hlo);
  setLayout(vlo);
}
