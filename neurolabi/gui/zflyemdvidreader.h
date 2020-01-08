#ifndef ZFLYEMDVIDREADER_H
#define ZFLYEMDVIDREADER_H

#include "dvid/zdvidreader.h"
#include "flyem/zflyembodyannotation.h"

/*!
 * \brief The class of reading flyem data from DVID (Deprecated)
 *
 */
class ZFlyEmDvidReader : public ZDvidReader
{
public:
  ZFlyEmBodyAnnotation readAnnotation(int bodyId);
  ZStack* readThumbnail(int bodyId);
  QStringList readSynapseList();
  ZJsonObject readSynapseAnnotation(const QString &name);
};

#endif // ZFLYEMDVIDREADER_H
