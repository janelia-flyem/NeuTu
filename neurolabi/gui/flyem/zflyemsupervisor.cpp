#include "zflyemsupervisor.h"
#include "neutube.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "zrandomgenerator.h"

ZFlyEmSupervisor::ZFlyEmSupervisor(QObject *parent) :
  QObject(parent)
{
  m_userName = NeuTube::GetUserName();
}


void ZFlyEmSupervisor::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

bool ZFlyEmSupervisor::checkIn(uint64_t bodyId)
{
  static ZRandomGenerator generator;

  return generator.rndint(2) > 0;
}

bool ZFlyEmSupervisor::checkOut(uint64_t bodyId)
{
  static ZRandomGenerator generator;

  return generator.rndint(2) > 0;
}
