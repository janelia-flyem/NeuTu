#ifndef ZGROUPVISWIDGET_H
#define ZGROUPVISWIDGET_H

#include <QWidget>
#include <QSet>

/*!
 * \brief The widget class for updating visibility with child widgets
 *
 * The class is used to provide the visibility hint of a widget. The hint is
 * also called visibility assumption (VA), which is determined by the following rules:
 * 1. First, a ZGroupVisWidget object has a default visibility assumption (DVA).
 *    If the DVA is false, then the widget is assumed to be hidden.
 * 2. If the DVA is true, then the widget checks its child widget to see if
 *    any of them is assumed to be visible. In this case, the VA of a child
 *    widget is managed by the parent widget fully, or partially if the child has
 *    its own VA.
 */
class ZGroupVisWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZGroupVisWidget(QWidget *parent = nullptr);

  /*!
   * \brief Update the visiblity of the widget
   *
   * When its default visibility is true, set the widget to visible if any of
   * its wigets is visible. Otherwise, hide the widget.
   */
  void updateVisibility();

  /*!
   * \brief Get the default visibility
   *
   * The default visibility is the first-level visibility check for the widget.
   * The widget is always assumed to be hidden if the default visibility is false.
   */
  bool isDefaultVisible() const;

  /*!
   * \brief Set the default visibility
   */
  void setDefaultVisible(bool on);

  /*!
   * \brief Toggle the default visibility
   *
   * It flips the default visibility status.
   */
  void toggleDefaultVisible();

  /*!
   * \brief Assume a widget is visible or not
   *
   * This records the assumed visiblity represented by \a visible for \a widget,
   * whose actual visibility will be neither checked nor modified.
   *
   * Note: A widget that has its own visibility assumption will not necessay
   * become assumed to be visible.
   */
  void assumeVisible(const QWidget *widget, bool visible);

  /*!
   * \brief Get the visible assumption of a child widget.
   *
   * It is independent of the default visibility. A widget is assumed to be
   * visible if:
   * 1. it is recorded to be visible and does not have its own visibility
   * assumption; or
   * 2. it is recored to be visible and its own visibility is assumed to be true.
   */
  bool assumingVisible(const QWidget *widget) const;

  /*!
   * \brief Get the visible assumption of the group widget.
   *
   * The widget is assumed visible iff its default visibility is true and
   * any of its child is asssumed to be visible.
   */
  bool assumingVisible() const;

  /*!
   * \brief Set the visibility of a widget
   *
   * It does not only update the visibility assumption but also set the
   * actual visbility.
   */
  void setWidgetVisible(QWidget *widget, bool visible);

signals:

private:
  bool m_defaultVisible = true;
  QSet<const QWidget*> m_visibleWidgets;
};

#endif // ZGROUPVISWIDGET_H
