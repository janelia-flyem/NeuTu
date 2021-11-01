#ifndef ZDVIDTARGETBUILDER_H
#define ZDVIDTARGETBUILDER_H

#include "zdvidtarget.h"
#include "zdviddata.h"

class ZDvidTargetRoiBuilder;
class ZDvidTargetMainBuilder;

class ZDvidTargetBuilderBase {
public:
  ZDvidTargetBuilderBase(ZDvidTarget &target) : m_targetRef(target) {};

  operator ZDvidTarget() const;

  ZDvidTargetRoiBuilder roi() const;
  ZDvidTargetMainBuilder main() const;

protected:
  ZDvidTarget &m_targetRef;
};

class ZDvidTargetMainBuilder : public ZDvidTargetBuilderBase
{
public:
  ZDvidTargetMainBuilder(ZDvidTarget &target);

  /*!
   * \brief Set the server of the target
   * \param server it can be in the [schem://]address[:port] format. An
   *   unrecognized format will set the server to empty.
   */
  ZDvidTargetMainBuilder& on(const std::string &server);
  ZDvidTargetMainBuilder& withUuid(const std::string &uuid);
  ZDvidTargetMainBuilder& withSegmentation(const std::string &name);
  ZDvidTargetMainBuilder& withGrayscale(const std::string &name);
};

class ZDvidTargetBuilder : public ZDvidTargetMainBuilder
{
public:
  ZDvidTargetBuilder();

  /*!
   * \brief Set the data of the target
   *
   * It sets the name of the data with a role \a role to \a name. If \a role
   * is not supported, nothing will be done.
   */
//  ZDvidTargetBuilder& withData(ZDvidData::ERole role, const std::string &name);

private:
  ZDvidTarget m_target;
};

class ZDvidTargetRoiBuilder : public ZDvidTargetBuilderBase
{
public:
  ZDvidTargetRoiBuilder(ZDvidTarget &target);
  ZDvidTargetRoiBuilder& add(const std::string &name);
  ZDvidTargetRoiBuilder& set(const std::string &name);
};

#endif // ZDVIDTARGETBUILDER_H
