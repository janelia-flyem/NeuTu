#include "zgroupviswidget.h"

#include "common/debug.h"
#include "qt/gui/utilities.h"

ZGroupVisWidget::ZGroupVisWidget(QWidget *parent) : QWidget(parent)
{

}

void ZGroupVisWidget::setDefaultVisible(bool on)
{
  m_defaultVisible = on;
  updateVisibility();
}

bool ZGroupVisWidget::isDefaultVisible() const
{
  return m_defaultVisible;
}

void ZGroupVisWidget::toggleDefaultVisible()
{
  m_defaultVisible = !m_defaultVisible;
}

bool ZGroupVisWidget::assumingVisible() const
{
  bool visible = false;
  if (m_defaultVisible && !m_visibleWidgets.isEmpty()) {
    visible = neutu::AnyWidgetInLayout(layout(), [&](const QWidget *widget) {
      return assumingVisible(widget);
    });
  }

  return visible;
}

void ZGroupVisWidget::updateVisibility()
{
  setVisible(assumingVisible());
}

void ZGroupVisWidget::assumeVisible(const QWidget *widget, bool visible)
{
  if (widget) {
    if (visible) {
      m_visibleWidgets.insert(widget);
    } else {
      m_visibleWidgets.remove(widget);
    }
    HLDEBUG("groupvis") << widget->metaObject()->className() << " assumed "
                        << (visible ? "visible" : "hidden") << std::endl;
  }
}

bool ZGroupVisWidget::assumingVisible(const QWidget *widget) const
{
  if (m_visibleWidgets.contains(widget)) {
    const ZGroupVisWidget *childGroup =
        qobject_cast<const ZGroupVisWidget*>(widget);
    if (childGroup) {
      return childGroup->assumingVisible();
    } else {
      return true;
    }
  }

  return false;
}

void ZGroupVisWidget::setWidgetVisible(QWidget *widget, bool visible)
{
  if (widget) {
    widget->setVisible(visible);
    assumeVisible(widget, visible);
  }
}
