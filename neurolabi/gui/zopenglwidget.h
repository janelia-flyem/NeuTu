#ifndef ZOPENGLWIDGET_H
#define ZOPENGLWIDGET_H

#include <QOpenGLWidget>

class ZOpenGLWidget : public QOpenGLWidget
{
  Q_OBJECT
public:
  using QOpenGLWidget::QOpenGLWidget;

  // QOpenGLWidget interface
protected:
  virtual void initializeGL() override;

signals:
  void openGLContextInitialized();
};

#endif // ZOPENGLWIDGET_H
