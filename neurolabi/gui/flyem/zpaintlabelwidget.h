#ifndef ZPAINTLABELWIDGET_H
#define ZPAINTLABELWIDGET_H

#include <QWidget>

class ZClickableColorLabel;

class ZPaintLabelWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZPaintLabelWidget(QWidget *parent = 0);

signals:

private:
  void init(int maxLabel);
  ZClickableColorLabel* makeColorWidget(const QColor &color, int label);

public slots:

private:
  const static int m_maxLabel;
  ZClickableColorLabel *m_colorLabel;
};

#endif // ZPAINTLABELWIDGET_H
