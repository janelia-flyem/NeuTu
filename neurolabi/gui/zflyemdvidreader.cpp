#include "zflyemdvidreader.h"

ZFlyEmBodyAnnotation ZFlyEmDvidReader::readAnnotation(int bodyId)
{
  QByteArray data = readKeyValue("annotations", QString("%1").arg(bodyId));
  ZFlyEmBodyAnnotation annotation;
  annotation.loadJsonString(QString(data).toStdString());

  annotation.setBodyId(bodyId);

  return annotation;
}
