#ifndef ZPAINTLABELWIDGET_H
#define ZPAINTLABELWIDGET_H

#include <QWidget>

class ZClickableColorLabel;
class QGroupBox;

class ZPaintLabelWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZPaintLabelWidget(QWidget *parent = 0);
  void setTitle(const QString &title);

  QSize minimumSizeHint() const;

signals:

private:
  void init(int maxLabel);
  ZClickableColorLabel* makeColorWidget(const QColor &color, int label);

public slots:

private:
  const static int m_maxLabel;
  ZClickableColorLabel *m_colorLabel;
  QGroupBox *m_groupBox = NULL;
};

#endif // ZPAINTLABELWIDGET_H
