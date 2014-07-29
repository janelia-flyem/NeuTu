#ifndef ZDVIDSERVERWIDGET_H
#define ZDVIDSERVERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include "zwidgetfactory.h"

class ZDvidServerWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZDvidServerWidget(QWidget *parent = 0);

  void createWidgets();

signals:

public slots:

private:
  ZLabeledEditWidget *m_addressWidget;
  ZLabeledEditWidget *m_portWidget;
  ZLabeledEditWidget *m_uuidWidget;
};

#endif // ZDVIDSERVERWIDGET_H
