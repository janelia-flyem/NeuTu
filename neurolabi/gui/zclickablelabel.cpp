#include "zclickablelabel.h"

#include "zglmutils.h"
#include "zcolormap.h"
#include "z3dtransferfunction.h"
#include "widgets/znumericparameter.h"
#include <QMouseEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <QColorDialog>
#include <QPainter>

ZClickableLabel::ZClickableLabel(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
}

ZClickableLabel::~ZClickableLabel()
{}

void ZClickableLabel::mousePressEvent(QMouseEvent* ev)
{
  if (ev->button() == Qt::LeftButton) {
    labelClicked();
  }
}

bool ZClickableLabel::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip) {
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
    QRect tipRect;
    QString tipText;
    if (getTip(helpEvent->pos(), &tipRect, &tipText)) {
      QToolTip::showText(
        helpEvent->globalPos(), tipText, this, tipRect);
    } else {
      QToolTip::hideText();
    }
  }
  return QWidget::event(event);
}

void ZClickableLabel::labelClicked()
{
  emit clicked();
}

ZClickableColorLabel::ZClickableColorLabel(ZVec4Parameter* color, QWidget* parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_vec4Color(color)
{
#if __cplusplus > 201103L
  connect(m_vec4Color, &ZVec4Parameter::valueChanged, this, qOverload<>(&ZClickableColorLabel::update));
#else
  connect(m_vec4Color, &ZVec4Parameter::valueChanged, this, QOverload<>::of(&ZClickableColorLabel::update));
#endif

  initColorDlg();
}

ZClickableColorLabel::ZClickableColorLabel(ZVec3Parameter* color, QWidget* parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_vec3Color(color)
{
#if __cplusplus > 201103L
  connect(m_vec3Color, &ZVec3Parameter::valueChanged, this, qOverload<>(&ZClickableColorLabel::update));
#else
  connect(m_vec3Color, &ZVec3Parameter::valueChanged, this, QOverload<>::of(&ZClickableColorLabel::update));
#endif

  initColorDlg();
}

ZClickableColorLabel::ZClickableColorLabel(ZDVec4Parameter* color, QWidget* parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_dvec4Color(color)
{
#if __cplusplus > 201103L
  connect(m_dvec4Color, &ZDVec4Parameter::valueChanged, this, qOverload<>(&ZClickableColorLabel::update));
#else
  connect(m_dvec4Color, &ZDVec4Parameter::valueChanged, this, QOverload<>::of(&ZClickableColorLabel::update));
#endif

  initColorDlg();
}

ZClickableColorLabel::ZClickableColorLabel(ZDVec3Parameter* color, QWidget* parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_dvec3Color(color)
{
#if __cplusplus > 201103L
  connect(m_dvec3Color, &ZDVec3Parameter::valueChanged, this, qOverload<>(&ZClickableColorLabel::update));
#else
  connect(m_dvec3Color, &ZDVec3Parameter::valueChanged, this, QOverload<>::of(&ZClickableColorLabel::update));
#endif

  initColorDlg();
}

ZClickableColorLabel::~ZClickableColorLabel()
{
#ifdef _DEBUG_2
  qDebug() << "ZClickableColorLabel " << this << " distroyed";
#endif

  m_colorDlg->reject();
  m_colorDlg->setParent(nullptr);
  m_colorDlg->deleteLater();
}

void ZClickableColorLabel::initColorDlg()
{
  m_colorDlg = new QColorDialog(this);
  connect(m_colorDlg, SIGNAL(accepted()), this, SLOT(updateColor()));
//  m_colorDlg->setOption(QColorDialog::NoButtons);
//  connect(m_colorDlg, SIGNAL(currentColorChanged(QColor)),
//          this, SLOT(setColor(QColor)), Qt::QueuedConnection);
}

void ZClickableColorLabel::paintEvent(QPaintEvent* e)
{
  if (!m_vec4Color && !m_vec3Color && !m_dvec4Color && !m_dvec3Color) {
    QWidget::paintEvent(e); // clear the widget
    return;
  }

  QColor labelColor = toQColor();
  QPainter painter(this);
  painter.setBrush(labelColor);
  painter.drawRect(1, 1, rect().width() - 2, rect().height() - 2);
  if (m_vec4Color) {
    double gray = .299*labelColor.redF() + .587*labelColor.greenF() +
        .114*labelColor.blueF();
    if (gray > 0.5) {
      painter.setPen(QColor(0, 0, 0));
    } else {
      painter.setPen(QColor(255, 255, 255));
    }
    painter.drawText(rect(), Qt::AlignCenter, m_vec4Color->name());
  }
}

QSize ZClickableColorLabel::minimumSizeHint() const
{
  return QSize(m_width, m_height);
}

bool ZClickableColorLabel::getTip(const QPoint& p, QRect* r, QString* s)
{
  if (!m_vec4Color && !m_vec3Color && !m_dvec4Color && !m_dvec3Color)
    return false;

  if (contentsRect().contains(p)) {
    *r = contentsRect();
    *s = toQColor().name();
    return true;
  }

  return false;
}

void ZClickableColorLabel::setColor(const QColor &color)
{
  QMutexLocker lock(m_syncMutex);
  if (color.isValid()) {
    fromQColor(color);
  }
}

void ZClickableColorLabel::updateColor()
{
  setColor(m_colorDlg->currentColor());
}


void ZClickableColorLabel::labelClicked()
{
  if (m_isClickable) {
    if (m_colorDlg) {
#ifdef _DEBUG_
      qDebug() << "ZClickableColorLabel::labelClicked in " << this;
#endif
//      m_colorDlg->exec();
      m_colorDlg->show();
      m_colorDlg->raise();
    } else {
      QColorDialog dlg(toQColor());
      if (dlg.exec()) {
        setColor(dlg.selectedColor());
      }
    }
  }
}

QColor ZClickableColorLabel::toQColor()
{
  if (m_vec4Color) {
    return QColor(static_cast<int>(m_vec4Color->get().r * 255.f),
                  static_cast<int>(m_vec4Color->get().g * 255.f),
                  static_cast<int>(m_vec4Color->get().b * 255.f));
  } else if (m_vec3Color) {
    return QColor(static_cast<int>(m_vec3Color->get().r * 255.f),
                  static_cast<int>(m_vec3Color->get().g * 255.f),
                  static_cast<int>(m_vec3Color->get().b * 255.f));
  } else if (m_dvec4Color) {
    return QColor(static_cast<int>(m_dvec4Color->get().r * 255.f),
                  static_cast<int>(m_dvec4Color->get().g * 255.f),
                  static_cast<int>(m_dvec4Color->get().b * 255.f));
  } else if (m_dvec3Color) {
    return QColor(static_cast<int>(m_dvec3Color->get().r * 255.f),
                  static_cast<int>(m_dvec3Color->get().g * 255.f),
                  static_cast<int>(m_dvec3Color->get().b * 255.f));
  } else {
    return QColor(0, 0, 0);
  }
}

void ZClickableColorLabel::fromQColor(const QColor& col)
{
  if (m_vec4Color)
    m_vec4Color->set(glm::vec4(col.redF(), col.greenF(), col.blueF(), m_vec4Color->get().a));
  if (m_vec3Color)
    m_vec3Color->set(glm::vec3(col.redF(), col.greenF(), col.blueF()));
  if (m_dvec4Color)
    m_dvec4Color->set(glm::dvec4(col.redF(), col.greenF(), col.blueF(), m_dvec4Color->get().a));
  if (m_dvec3Color)
    m_dvec3Color->set(glm::dvec3(col.redF(), col.greenF(), col.blueF()));
}

ZClickableColorMapLabel::ZClickableColorMapLabel(ZColorMapParameter* colorMap, QWidget* parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_colorMap(colorMap)
{
#if __cplusplus > 201103L
  connect(m_colorMap, &ZColorMapParameter::valueChanged, this, qOverload<>(&ZClickableColorMapLabel::update));
#else
  connect(m_colorMap, &ZColorMapParameter::valueChanged,
          this, QOverload<>::of(&ZClickableColorMapLabel::update));
#endif
}

void ZClickableColorMapLabel::paintEvent(QPaintEvent* e)
{
  if (!m_colorMap) {
    QWidget::paintEvent(e); // clear the widget
    return;
  }

  QPainter painter(this);

  for (int x = contentsRect().left(); x <= contentsRect().right(); ++x) {
    painter.setPen(m_colorMap->get().fractionMappedQColor((x * 1. - contentsRect().left()) / contentsRect().width()));
    painter.drawLine(x, contentsRect().top(), x, contentsRect().bottom());
  }
}

QSize ZClickableColorMapLabel::minimumSizeHint() const
{
  return QSize(255, 33);
}

bool ZClickableColorMapLabel::getTip(const QPoint& p, QRect* r, QString* s)
{
  if (!m_colorMap)
    return false;

  if (contentsRect().contains(p)) {
    r->setCoords(p.x(), contentsRect().top(),
                 p.x(), contentsRect().bottom());
    QColor color = m_colorMap->get().fractionMappedQColor(
      (p.x() * 1. - contentsRect().left()) / contentsRect().width());
    *s = color.name();
    return true;
  }

  return false;
}

ZClickableTransferFunctionLabel::ZClickableTransferFunctionLabel(Z3DTransferFunctionParameter* transferFunc,
                                                                 QWidget* parent, Qt::WindowFlags f)
  : ZClickableLabel(parent, f)
  , m_transferFunction(transferFunc)
{
#if __cplusplus > 201103L
  connect(m_transferFunction, &Z3DTransferFunctionParameter::valueChanged,
          this, qOverload<>(&ZClickableTransferFunctionLabel::update));
#else
  connect(m_transferFunction, &Z3DTransferFunctionParameter::valueChanged,
          this, QOverload<>::of(&ZClickableTransferFunctionLabel::update));
#endif
}

void ZClickableTransferFunctionLabel::paintEvent(QPaintEvent* /*e*/)
{
  QPainter painter(this);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  QColor color1(0, 0, 0);
  QColor color2(255, 255, 255);
  qreal width = contentsRect().width() / 20.;
  qreal height = contentsRect().height() / 4.;
  for (int i = 0; i < 20; ++i) {
    if (i % 2 == 0) {
      painter.fillRect(QRectF(contentsRect().left() + i * width, contentsRect().top(), width, height), color2);
      painter.fillRect(
        QRectF(contentsRect().left() + i * width, 0.25 * (contentsRect().top() + contentsRect().bottom()), width,
               height), color1);
    } else {
      painter.fillRect(QRectF(contentsRect().left() + i * width, contentsRect().top(), width, height), color1);
      painter.fillRect(
        QRectF(contentsRect().left() + i * width, 0.25 * (contentsRect().top() + contentsRect().bottom()), width,
               height), color2);
    }
  }

  if (m_transferFunction) {
    for (int x = contentsRect().left(); x <= contentsRect().right(); ++x) {
      double fraction = (x * 1. - contentsRect().left()) / contentsRect().width();
      QColor color = m_transferFunction->get().mappedQColor(fraction);
      painter.setPen(color);
      painter.drawLine(x, contentsRect().top(), x, contentsRect().bottom() * 0.5);
      color.setAlpha(255);
      painter.setPen(color);
      painter.drawLine(x, contentsRect().bottom() * 0.5, x, contentsRect().bottom());
    }
  }
}

QSize ZClickableTransferFunctionLabel::minimumSizeHint() const
{
  return QSize(255, 33);
}

bool ZClickableTransferFunctionLabel::getTip(const QPoint& p, QRect* r, QString* s)
{
  if (!m_transferFunction)
    return false;

  if (contentsRect().contains(p)) {
    r->setCoords(p.x(), contentsRect().top(),
                 p.x(), contentsRect().bottom());
    QColor color = m_transferFunction->get().fractionMappedQColor(
      (p.x() * 1. - contentsRect().left()) / contentsRect().width());
    *s = color.name();
    return true;
  }

  return false;
}
