#ifndef ZSKELETONIZESERVICE_H
#define ZSKELETONIZESERVICE_H

#include <QString>
#include "dvid/zdvidtarget.h"

class ZSkeletonizeService
{
public:
  ZSkeletonizeService();

  void callService(const ZDvidTarget &target, int bodyId);

private:
  QString m_server;
};

#endif // ZSKELETONIZESERVICE_H
