#include "zstackdockeyprocessor.h"
#include <QKeyEvent>

ZStackDocKeyProcessor::ZStackDocKeyProcessor(QObject *parent) : QObject(parent)
{

}

bool ZStackDocKeyProcessor::processKeyEvent(QKeyEvent */*event*/)
{
  return false;
}
