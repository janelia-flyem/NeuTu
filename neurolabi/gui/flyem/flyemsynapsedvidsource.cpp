#include "flyemsynapsedvidsource.h"

#include "geometry/zintcuboid.h"
#include "flyemdatareader.h"
#include "flyemdatawriter.h"

FlyEmSynapseDvidSource::FlyEmSynapseDvidSource()
{

}

void FlyEmSynapseDvidSource::setDvidTarget(const ZDvidTarget &target)
{
  m_writer.open(target);
  m_writer.getDvidReader().setVerbose(false);
}

std::vector<ZDvidSynapse>
FlyEmSynapseDvidSource::getData(const ZIntCuboid &box) const
{
  return FlyEmDataReader::ReadSynapse(
        m_writer.getDvidReader(), box,
        dvid::EAnnotationLoadMode::PARTNER_LOCATION);
}

ZIntCuboid FlyEmSynapseDvidSource::getRange() const
{
  return FlyEmDataReader::ReadTodoDataRange(m_writer.getDvidReader());
}

void FlyEmSynapseDvidSource::saveItem(const ZDvidSynapse &item)
{
  m_writer.writeSynapse(item);
  if (!m_writer.isStatusOk()) {
    throw std::runtime_error(
          "Failed to add synapse: " +
          m_writer.getStatusErrorMessage().toStdString());
  }
}

void FlyEmSynapseDvidSource::removeItem(const ZIntPoint &pos)
{
  m_writer.deleteSynapse(pos.getX(), pos.getY(), pos.getZ());
  if (!m_writer.isStatusOk()) {
    throw std::runtime_error(
          "Failed to remove synapse: " +
          m_writer.getStatusErrorMessage().toStdString());
  }
}

ZDvidSynapse FlyEmSynapseDvidSource::getItem(const ZIntPoint &pos) const
{
  return FlyEmDataReader::ReadSynapse(m_writer.getDvidReader(), pos);
}

void FlyEmSynapseDvidSource::moveItem(
    const ZIntPoint &from, const ZIntPoint &to)
{
  m_writer.moveSynapse(from, to);
  if (!m_writer.isStatusOk()) {
    throw std::runtime_error(
          "Failed to move synapse: " +
          m_writer.getStatusErrorMessage().toStdString());
  }
}

void FlyEmSynapseDvidSource::updatePartner(ZDvidSynapse *item) const
{
  FlyEmDataReader::UpdateSynapsePartner(m_writer.getDvidReader(), item);
//    item->updatePartnerProperty(m_writer.getDvidReader());
}

/*
void FlyEmSynapseDvidSource::updateItem(ZDvidSynapse *item) const
{
  FlyEmDataReader::UpdateSynapsePartner(m_writer.getDvidReader(), item);
}
*/
