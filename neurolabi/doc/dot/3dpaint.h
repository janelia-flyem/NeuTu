class ZInteractionEngine;
//class QObject;
class Z3DCanvas:QGraphicsScene;
class Z3DWindow;

connect ZInteractionEngine::SIGNAL(strokePainted), Z3DCanvas::strokePainted;
connect Z3DCanvas::SIGNAL(strokePainted()), Z3DWindow::addStrokeFrom3DPaint;
function ZInteractionEngine::commitData;

cmp Z3DCanvas::ZInteractionEngine;
cmp Z3DWindow::Z3DCanvas;


