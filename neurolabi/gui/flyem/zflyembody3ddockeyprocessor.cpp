#include "zflyembody3ddockeyprocessor.h"
#include <QKeyEvent>
#include "zflyembody3ddoc.h"

ZFlyEmBody3dDocKeyProcessor::ZFlyEmBody3dDocKeyProcessor(QObject *parent) :
  ZStackDocKeyProcessor(parent)
{

}

bool ZFlyEmBody3dDocKeyProcessor::processKeyEvent(QKeyEvent *event)
{
  bool processed = false;

  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    switch (event->key()) {
    case Qt::Key_1:
      doc->setSeedType(1);
      processed = true;
      break;
    case Qt::Key_2:
      doc->setSeedType(2);
      processed = true;
      break;
    }
  }

  return processed;
}
