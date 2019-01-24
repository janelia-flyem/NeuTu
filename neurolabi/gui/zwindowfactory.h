#ifndef ZWINDOWFACTORY_H
#define ZWINDOWFACTORY_H

#include <QWidget>
#include <QString>
#include <QMap>

#include "core/zsharedpointer.h"
//#include "zstackdoc.h"
//#include "z3dwindow.h"
#include "z3ddef.h"
#include "z3dview.h"

class ZStackFrame;
class QDialog;
class ZScalableStack;
class ZSwcTree;
class ZStackDoc;
class Z3DWindow;

/*!
 * \brief The factory class of creating windows
 */
class ZWindowFactory
{
public:
  ZWindowFactory();
  virtual ~ZWindowFactory();



  Z3DWindow* make3DWindow(ZStackDoc *doc,
                          Z3DView::EInitMode mode = Z3DView::EInitMode::INIT_NORMAL);
  Z3DWindow* open3DWindow(ZStackDoc *doc,
                          Z3DView::EInitMode mode = Z3DView::EInitMode::INIT_NORMAL);

  Z3DWindow* make3DWindow(ZSharedPointer<ZStackDoc> doc,
                          Z3DView::EInitMode mode = Z3DView::EInitMode::INIT_NORMAL);
  static Z3DWindow* Open3DWindow(
      ZStackFrame *frame, Z3DView::EInitMode mode = Z3DView::EInitMode::INIT_NORMAL);


  Z3DWindow* make3DWindow(ZScalableStack *stack);

  void setWindowTitle(const QString &title);
  void setParentWidget(QWidget *parentWidget);
  void setWindowGeometry(const QRect &rect);
  void setWindowType(neutube3d::EWindowType type);

  void setVisible(neutube3d::ERendererLayer layer, bool visible);

  inline void setControlPanelVisible(bool visible) {
    m_showControlPanel = visible;
  }

  inline void setObjectViewVisible(bool visible) {
    m_showObjectView = visible;
  }

  inline void setStatusBarVisible(bool visible) {
    m_showStatusBar = visible;
  }

  inline bool isControlPanelVisible() const {
    return m_showControlPanel;
  }

  inline bool isObjectViewVisible() const {
    return m_showObjectView;
  }

  inline bool isStatusBarVisible() const {
    return m_showStatusBar;
  }

  inline void setVolumeRenderMode(neutube3d::EVolumeRenderingMode mode) {
    m_volumeMode = mode;
  }

  void setDeleteOnClose(bool on) { m_deleteOnClose = on; }

protected:
  virtual void configure(Z3DWindow *window);

private:
  void init();

private:
  QWidget *m_parentWidget = NULL;
  QString m_windowTitle;
  QRect m_windowGeometry;

  bool m_showVolumeBoundBox = false;
  bool m_showControlPanel = true;
  bool m_showObjectView = true;
  bool m_showStatusBar = false;
  neutube3d::EVolumeRenderingMode m_volumeMode = neutube3d::EVolumeRenderingMode::VR_AUTO;

  bool m_deleteOnClose = false;

  neutube3d::EWindowType m_windowType = neutube3d::EWindowType::GENERAL;

  QMap<neutube3d::ERendererLayer, bool> m_layerVisible;
};

#endif // ZWINDOWFACTORY_H
