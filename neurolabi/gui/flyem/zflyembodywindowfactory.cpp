#include "zflyembodywindowfactory.h"
#include "z3dgraphfilter.h"
#include "z3dswcfilter.h"

ZFlyEmBodyWindowFactory::ZFlyEmBodyWindowFactory()
{
}

void ZFlyEmBodyWindowFactory::configure(Z3DWindow *window)
{
  if (window != NULL) {
    window->getGraphFilter()->setStayOnTop(false);

    window->getSwcFilter()->setColorMode("Intrinsic");
    window->getSwcFilter()->setRenderingPrimitive("Sphere");
    window->getSwcFilter()->setStayOnTop(false);
  }
}
