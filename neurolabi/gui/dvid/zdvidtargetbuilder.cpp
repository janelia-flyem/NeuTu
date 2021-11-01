#include "zdvidtargetbuilder.h"

#include <regex>

ZDvidTargetBuilderBase::operator ZDvidTarget() const
{
  return m_targetRef;
}

ZDvidTargetRoiBuilder ZDvidTargetBuilderBase::roi() const
{
  return ZDvidTargetRoiBuilder(m_targetRef);
}

ZDvidTargetMainBuilder ZDvidTargetBuilderBase::main() const
{
  return ZDvidTargetMainBuilder(m_targetRef);
}

ZDvidTargetBuilder::ZDvidTargetBuilder() : ZDvidTargetMainBuilder(m_target)
{

}

ZDvidTargetMainBuilder::ZDvidTargetMainBuilder(ZDvidTarget &target)
  : ZDvidTargetBuilderBase(target)
{

}

ZDvidTargetMainBuilder& ZDvidTargetMainBuilder::on(const std::string &server)
{
  m_targetRef.setServer(server);
  return *this;
}

ZDvidTargetMainBuilder& ZDvidTargetMainBuilder::withUuid(const std::string &uuid)
{
  m_targetRef.setUuid(uuid);

  return *this;
}

ZDvidTargetMainBuilder& ZDvidTargetMainBuilder::withSegmentation(const std::string &name)
{
  m_targetRef.setSegmentationName(name);

  return *this;
}

ZDvidTargetMainBuilder& ZDvidTargetMainBuilder::withGrayscale(const std::string &name)
{
  m_targetRef.setGrayScaleName(name);

  return *this;
}

ZDvidTargetRoiBuilder::ZDvidTargetRoiBuilder(ZDvidTarget &target)
  : ZDvidTargetBuilderBase(target)
{

}

ZDvidTargetRoiBuilder& ZDvidTargetRoiBuilder::add(const std::string &name)
{
  m_targetRef.addRoiName(name);

  return *this;
}

ZDvidTargetRoiBuilder& ZDvidTargetRoiBuilder::set(const std::string &name)
{
  m_targetRef.setRoiName(name);

  return *this;
}
