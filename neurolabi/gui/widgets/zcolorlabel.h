#ifndef ZCOLORLABEL_H
#define ZCOLORLABEL_H

#include <QLabel>

class ZColorLabel : public QLabel
{
  Q_OBJECT
public:
  explicit ZColorLabel(QWidget *parent = 0);

  inline void setWidth(int width) {
    m_width = width;
  }

  inline void setHeight(int height) {
    m_height = height;
  }

  inline void setClickable(bool state) {
    m_clickable = state;
  }

  void setColor(const QColor &col);

signals:

public slots:

protected:
  virtual QSize minimumSizeHint() const;

private:
  int m_width;
  int m_height;
  bool m_clickable;
};

#endif // ZCOLORLABEL_H
