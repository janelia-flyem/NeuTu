#ifndef FLYEMDATACONFIG_H
#define FLYEMDATACONFIG_H

#include "zcontrastprotocol.h"
#include "zflyembodyannotationprotocol.h"

class ZJsonObject;

/*!
 * \brief The class of flyem data configuration.
 */
class FlyEmDataConfig
{
public:
  FlyEmDataConfig();

  void loadJsonObject(const ZJsonObject &obj);
  void loadContrastProtocol(const ZJsonObject &obj);
  void loadBodyStatusProtocol(const ZJsonObject &obj);

  const ZContrastProtocol& getContrastProtocol() const {
    return m_contrastProtocol;
  }

  const ZFlyEmBodyAnnotationProtocol& getBodyStatusProtocol() const {
    return m_bodyStatusProtocol;
  }

  void print() const;

public:
  static const char* KEY_CONTRAST;
  static const char* KEY_BODY_STATUS;

private:
  ZContrastProtocol m_contrastProtocol;
  ZFlyEmBodyAnnotationProtocol m_bodyStatusProtocol;

};

#endif // FLYEMDATACONFIG_H
