#ifndef ZHWIDGET_H
#define ZHWIDGET_H

#include <QVector>
#include <QWidget>

#include "zgroupviswidget.h"

class QHBoxLayout;

class ZHWidget : public ZGroupVisWidget
{
  Q_OBJECT
public:
  explicit ZHWidget(int layoutCount, QWidget *parent = nullptr);

  /*!
   * \brief Add a widget to a segment
   *
   * The new widget is added to the last position of the segment at \a layoutIndex.
   * Nothing will be done if \a layoutIndex is out of range.
   */
  void addWidget(QWidget *widget, int layoutIndex);

  /*!
   * \brief Add a layout to a segment
   *
   * All widgets in \a layout will be assumed to be visible. Nothing will be
   * done if \a layoutIndex is out of range.
   */
  void addLayout(QLayout *layout, int layoutIndex);

  /*!
   * \brief Remove a child widget.
   *
   * It does not delete \a widget.
   */
  void removeWidget(QWidget *widget);

private:
  QHBoxLayout* getLayout(int layoutIndex) const;

signals:

private:
  QVector<QHBoxLayout*> m_childLayout;
};

#endif // ZHWIDGET_H
