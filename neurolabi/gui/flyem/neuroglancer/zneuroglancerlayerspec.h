#ifndef ZNEUROGLANCERLAYERSPEC_H
#define ZNEUROGLANCERLAYERSPEC_H

#include <string>

class ZJsonObject;

class ZNeuroglancerLayerSpec
{
public:
  ZNeuroglancerLayerSpec();

  std::string getName() const {
    return m_name;
  }

  void setSource(const std::string &source) {
    m_source = source;
  }

  void setType(const std::string &type) {
    m_type = type;
  }

  void setName(const std::string &name) {
    m_name = name;
  }

  virtual ZJsonObject toJsonObject() const;

//  std::string toPathString() const;

private:
  std::string m_source;
  std::string m_type;
  std::string m_name;
};

#endif // ZNEUROGLANCERLAYERSPEC_H
