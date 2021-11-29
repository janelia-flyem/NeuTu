#ifndef ZH3WIDGET_H
#define ZH3WIDGET_H

#include <QWidget>

#include "qt/core/defs.h"
#include "zhwidget.h"

class QHBoxLayout;

/*!
 * \brief Horizontal widget with three segments (left, center, right)
 */
class ZH3Widget : public ZHWidget
{
  Q_OBJECT
public:
  explicit ZH3Widget(QWidget *parent = nullptr);

  /*!
   * \brief Add a widget to the segment
   *
   * The new widget is added to the last position of the segment.
   */
  void addWidget(QWidget *widget, neutu::EH3Layout seg);
  void addLayout(QLayout *layout, neutu::EH3Layout seg);

private:
  int getLayoutIndex(neutu::EH3Layout seg) const;

signals:
};

#endif // ZH3WIDGET_H
