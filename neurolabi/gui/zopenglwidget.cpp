#include "zopenglwidget.h"

void ZOpenGLWidget::initializeGL()
{
  emit openGLContextInitialized();
}
