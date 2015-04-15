#ifndef ZWINDOWFACTORY_H
#define ZWINDOWFACTORY_H

#include <QWidget>
#include <QString>
#include "zsharedpointer.h"
#include "zstackdoc.h"
#include "z3dwindow.h"

class ZStackFrame;
class QDialog;
class ZScalableStack;
class ZSwcTree;

/*!
 * \brief The factory class of creating windows
 */
class ZWindowFactory
{
public:
  ZWindowFactory();

  Z3DWindow* make3DWindow(ZStackDoc *doc,
                          Z3DWindow::EInitMode mode = Z3DWindow::NORMAL_INIT);
  Z3DWindow* open3DWindow(ZStackDoc *doc,
                          Z3DWindow::EInitMode mode = Z3DWindow::NORMAL_INIT);

  Z3DWindow* make3DWindow(ZSharedPointer<ZStackDoc> doc,
                          Z3DWindow::EInitMode mode = Z3DWindow::NORMAL_INIT);


  Z3DWindow* make3DWindow(ZScalableStack *stack);

  void setWindowTitle(const QString &title);
  void setParentWidget(QWidget *parentWidget);
  void setWindowGeometry(const QRect &rect);

  inline void setControlPanelVisible(bool visible) {
    m_showControlPanel = visible;
  }

  inline void setObjectViewVisible(bool visible) {
    m_showObjectView = visible;
  }

  inline bool isControlPanelVisible() const {
    return m_showControlPanel;
  }

  inline bool isObjectViewVisible() const {
    return m_showObjectView;
  }

private:
  QWidget *m_parentWidget;
  QString m_windowTitle;
  QRect m_windowGeometry;

  bool m_showVolumeBoundBox;
  bool m_showControlPanel;
  bool m_showObjectView;
};

#endif // ZWINDOWFACTORY_H
