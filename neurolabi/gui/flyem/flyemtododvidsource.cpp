#include "flyemtododvidsource.h"

#include "geometry/zintcuboid.h"

#include "flyemtodochunk.h"
#include "flyemdatareader.h"

FlyEmTodoDvidSource::FlyEmTodoDvidSource()
{
}

void FlyEmTodoDvidSource::setDvidTarget(const ZDvidTarget &target)
{
  m_writer.open(target);
}

std::vector<ZFlyEmToDoItem>
FlyEmTodoDvidSource::getData(const ZIntCuboid &box) const
{
  return FlyEmDataReader::ReadToDoItem(m_writer.getDvidReader(), box);
}
