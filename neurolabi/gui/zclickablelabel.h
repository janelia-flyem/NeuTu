#ifndef ZCLICKABLELABEL_H
#define ZCLICKABLELABEL_H

#include <QLabel>
#include <QMutex>

class QColorDialog;
class ZColorMapParameter;

class Z3DTransferFunctionParameter;

class ZVec4Parameter;

class ZVec3Parameter;

class ZDVec4Parameter;

class ZDVec3Parameter;

class ZClickableLabel : public QWidget
{
Q_OBJECT
public:
  explicit ZClickableLabel(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~ZClickableLabel();

  void setSyncMutex(QMutex *m) { m_syncMutex = m; }

signals:
  void clicked();


protected:
  virtual void mousePressEvent(QMouseEvent* ev) override;

  virtual bool event(QEvent* event) override;

  virtual bool getTip(const QPoint& p, QRect* r, QString* s) = 0;

  // default implement is emit the signal
  virtual void labelClicked();

  QMutex *m_syncMutex = nullptr;
};

class ZClickableColorLabel : public ZClickableLabel
{
  Q_OBJECT
public:
  explicit ZClickableColorLabel(ZVec4Parameter* color, QWidget* parent = 0, Qt::WindowFlags f = 0);

  explicit ZClickableColorLabel(ZVec3Parameter* color, QWidget* parent = 0, Qt::WindowFlags f = 0);

  explicit ZClickableColorLabel(ZDVec4Parameter* color, QWidget* parent = 0, Qt::WindowFlags f = 0);

  explicit ZClickableColorLabel(ZDVec3Parameter* color, QWidget* parent = 0, Qt::WindowFlags f = 0);

  ~ZClickableColorLabel();

  void setWidth(int w) { m_width = w; }
  void setHeight(int h) { m_height = h; }
  void setClickable(bool c) { m_isClickable = c; }

protected:
  virtual void paintEvent(QPaintEvent* e) override;

  virtual QSize minimumSizeHint() const override;

  ZVec4Parameter* m_vec4Color = nullptr;
  ZVec3Parameter* m_vec3Color = nullptr;
  ZDVec4Parameter* m_dvec4Color = nullptr;
  ZDVec3Parameter* m_dvec3Color = nullptr;

  virtual bool getTip(const QPoint& p, QRect* r, QString* s) override;

  virtual void labelClicked() override;
  void initColorDlg();


protected slots:
  void setColor(const QColor &color);
  void updateColor();

private:
  QColor toQColor();

  void fromQColor(const QColor& col);

private:
  int m_width = 50;
  int m_height = 33;
  int m_isClickable = true;
  QColorDialog *m_colorDlg =nullptr;
};

class ZClickableColorMapLabel : public ZClickableLabel
{
public:
  explicit ZClickableColorMapLabel(ZColorMapParameter* colorMap, QWidget* parent = nullptr,
                                   Qt::WindowFlags f = 0);

protected:
  virtual void paintEvent(QPaintEvent* e) override;

  virtual QSize minimumSizeHint() const override;

  ZColorMapParameter* m_colorMap;

  virtual bool getTip(const QPoint& p, QRect* r, QString* s) override;
};

class ZClickableTransferFunctionLabel : public ZClickableLabel
{
public:
  explicit ZClickableTransferFunctionLabel(Z3DTransferFunctionParameter* transferFunc, QWidget* parent = nullptr,
                                           Qt::WindowFlags f = 0);

protected:
  virtual void paintEvent(QPaintEvent* e) override;

  virtual QSize minimumSizeHint() const override;

  Z3DTransferFunctionParameter* m_transferFunction;

  virtual bool getTip(const QPoint& p, QRect* r, QString* s) override;
};


#endif // ZCLICKABLELABEL_H
