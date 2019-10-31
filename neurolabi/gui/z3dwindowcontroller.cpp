#include "z3dwindowcontroller.h"

#include "z3dwindow.h"
#include "z3dview.h"
#include "zstackdocselector.h"

Z3DWindowController::Z3DWindowController()
{

}

void Z3DWindowController::SetMeshVisible(Z3DWindow *window, bool on)
{
  window->setLayerVisible(neutu3d::ERendererLayer::MESH, on);
}

void Z3DWindowController::ToggleMeshVisible(Z3DWindow *window)
{
  window->setLayerVisible(neutu3d::ERendererLayer::MESH,
                          !window->isLayerVisible(neutu3d::ERendererLayer::MESH));
}

void Z3DWindowController::DeselectAllObject(Z3DWindow *window)
{
  ZStackDocSelector selector(window->getSharedDocument());
  selector.deselectAll();
}
