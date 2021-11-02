#include "flyemsynapsesource.h"

FlyEmSynapseSource::FlyEmSynapseSource()
{

}

ZIntPoint FlyEmSynapseSource::getBlockSize() const
{
  return ZIntPoint(128, 128, 128);
}
