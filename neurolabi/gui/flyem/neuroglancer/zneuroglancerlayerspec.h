#ifndef ZNEUROGLANCERLAYERSPEC_H
#define ZNEUROGLANCERLAYERSPEC_H

#include <string>

class ZJsonObject;

/*!
 * \brief The class for specifying a neuroglancer layer.
 *
 * A typical layer has the following three fields:
 *
 * name: name of the layer
 * type: type of the layer, which can be 'image', 'segmentation', 'annotation' and so on
 * source: source of the data
 */
class ZNeuroglancerLayerSpec
{
public:
  ZNeuroglancerLayerSpec();
  virtual ~ZNeuroglancerLayerSpec() {}

  std::string getName() const {
    return m_name;
  }

  std::string getType() const {
    return m_type;
  }

  std::string getSource() const {
    return m_source;
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

public:
  static const char* KEY_SOURCE;
  static const char* KEY_TYPE;
  static const char* KEY_NAME;

  static const char* TYPE_SEGMENTATION;
  static const char* TYPE_GRAYSCALE;
  static const char* TYPE_ANNOTATION;
  static const char* TYPE_SKELETON;

private:
  std::string m_source;
  std::string m_type;
  std::string m_name;
};

#endif // ZNEUROGLANCERLAYERSPEC_H
