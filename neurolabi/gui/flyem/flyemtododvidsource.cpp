#include "flyemtododvidsource.h"

#include "geometry/zintcuboid.h"

#include "flyemtodochunk.h"
#include "flyemdatareader.h"
#include "flyemdatawriter.h"

FlyEmTodoDvidSource::FlyEmTodoDvidSource()
{
}

FlyEmTodoDvidSource::~FlyEmTodoDvidSource()
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

ZIntCuboid FlyEmTodoDvidSource::getRange() const
{
  return FlyEmDataReader::ReadTodoDataRange(m_writer.getDvidReader());
}

void FlyEmTodoDvidSource::saveItem(const ZFlyEmToDoItem &item)
{
  FlyEmDataWriter::WriteTodoItem(m_writer, item);
  if (!m_writer.isStatusOk()) {
    throw std::runtime_error(
          "Failed to add todo: " +
          m_writer.getStatusErrorMessage().toStdString());
  }
}

void FlyEmTodoDvidSource::removeItem(const ZIntPoint &pos)
{
  m_writer.deleteToDoItem(pos.getX(), pos.getY(), pos.getZ());
  if (!m_writer.isStatusOk()) {
    throw std::runtime_error(
          "Failed to remove todo: " +
          m_writer.getStatusErrorMessage().toStdString());
  }
}
