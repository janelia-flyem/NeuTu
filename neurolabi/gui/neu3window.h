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
class ZFlyEmBody3dDoc;
class ZFlyEmProofDoc;

class Neu3Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Neu3Window(QWidget *parent = 0);
  ~Neu3Window();

  void initialize();
  void initOpenglContext();
  bool loadDvidTarget();

  ZFlyEmBody3dDoc* getBodyDocument() const;
  ZFlyEmProofDoc* getDataDocument() const;

public slots:
  void showSynapse(bool on);
  void showTodo(bool on);

  void removeBody(uint64_t bodyId);
  void addBody(uint64_t bodyId);

  void setBodySelection(const QSet<uint64_t> &bodySet);

protected:
  virtual void keyPressEvent(QKeyEvent *event);

private:
  void createDockWidget();
  void createToolBar();
  void connectSignalSlot();

private:
  Ui::Neu3Window *ui;

  Z3DCanvas *m_sharedContext = NULL;
  Z3DWindow *m_3dwin = NULL;
  ZFlyEmProofMvc *m_dataContainer = NULL;
  QToolBar *m_toolBar = NULL;
};

#endif // NEU3WINDOW_H
