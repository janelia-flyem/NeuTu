#include "zflyembodywindowfactory.h"
#include "z3dgraphfilter.h"
#include "z3dswcfilter.h"
#include "z3dpunctafilter.h"

ZFlyEmBodyWindowFactory::ZFlyEmBodyWindowFactory()
{
}

void ZFlyEmBodyWindowFactory::configure(Z3DWindow *window)
{
  if (window != NULL) {
    if (window->getGraphFilter() != NULL) {
      window->getGraphFilter()->setStayOnTop(false);
    }

    if (window->getSwcFilter() != NULL) {
      window->getSwcFilter()->setRenderingPrimitive("Sphere");
      window->getSwcFilter()->setColorMode("Intrinsic");
      window->getSwcFilter()->setStayOnTop(false);
    }

    if (window->getPunctaFilter() != NULL) {
      window->getPunctaFilter()->setStayOnTop(false);
      window->getPunctaFilter()->setColorMode("Original Point Color");
    }
  }
}
