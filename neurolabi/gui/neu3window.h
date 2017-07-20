#ifndef NEU3WINDOW_H
#define NEU3WINDOW_H

#include <QMainWindow>

namespace Ui {
class Neu3Window;
}

class Z3DWindow;
class Z3DCanvas;
class ZFlyEmProofMvc;
class QToolBar;

class Neu3Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Neu3Window(QWidget *parent = 0);
  ~Neu3Window();

  void initialize();
  void initOpenglContext();
  bool loadDvidTarget();

public slots:
  void showSynapse(bool on);
  void showTodo(bool on);

protected:
  virtual void keyPressEvent(QKeyEvent *event);

private:
  void createDockWidget();
  void createToolBar();
  void connectSignalSlot();

private:
  Ui::Neu3Window *ui;

  Z3DCanvas *m_sharedContext;
  Z3DWindow *m_3dwin;
  ZFlyEmProofMvc *m_dataContainer;
  QToolBar *m_toolBar;
};

#endif // NEU3WINDOW_H
