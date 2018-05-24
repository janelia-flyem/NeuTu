#ifndef ZFLYEMDATABUNDLE_H
#define ZFLYEMDATABUNDLE_H

#include <QColor>
#include <vector>
#include <string>
#include <map>

#include "zflyemneuron.h"
#include "flyem/zsynapseannotationarray.h"
#include "neutube.h"
#include "zflyemneuronarray.h"
#include "flyem/zflyemcoordinateconverter.h"

class ZDvidFilter;
class ZSwcTree;

class ZFlyEmDataBundle
{
public:
  ZFlyEmDataBundle();
  ~ZFlyEmDataBundle();

  enum EComponent {
    SYNAPSE_ANNOTATION, COLOR_MAP, ALL_COMPONENT
  };

  bool isDeprecated(EComponent comp) const;
  void deprecate(EComponent comp);
  void deprecateDependent(EComponent comp);

  bool loadJsonFile(const std::string &filePath);
  bool loadDvid(const ZDvidFilter &dvidFilter);

  std::string toSummaryString() const;
  std::string toDetailString() const;
  void print() const;

  std::string getModelPath(uint64_t bodyId) const;
  ZSwcTree* getModel(uint64_t bodyId) const;
  std::string getName(uint64_t bodyId) const;
  int getIdFromName(const std::string &name) const;

  bool hasNeuronName(const std::string &name) const;

  const ZFlyEmNeuronArray& getNeuronArray() const {
    return m_neuronArray;
  }

  ZFlyEmNeuronArray& getNeuronArray() {
    return m_neuronArray;
  }

  inline const std::string& getSource() const { return m_source; }

  //Return the pointer to the neuron with id <bodyId>. It returns NULL if no
  //such id is found.
  const ZFlyEmNeuron* getNeuron(uint64_t bodyId) const;
  ZFlyEmNeuron* getNeuron(uint64_t bodyId);
  const ZFlyEmNeuron* getNeuronFromName(const std::string &name) const;


  flyem::ZSynapseAnnotationArray *getSynapseAnnotation() const;
  void importSynpaseAnnotation(const std::string &filePath);
  std::map<int, QColor> *getColorMap() const;
  inline const std::map<std::string, double>& getMatchThresholdMap() const {
    return m_matchThreshold;
  }

  int countClass() const;
  int countNeuronByType(const std::string &className) const;
  //double getZResolution() const { return m_swcResolution[2]; }

  void updateNeuronConnection();

  /*!
   * \brief Get image resolution along a certain axis
   */
  double getImageResolution(neutube::EAxis axis);

  /*!
   * \brief Get SWC resolution along a certain axis
   */
  double getSwcResolution(neutube::EAxis axis);

  /*!
   * \brief Get source dimension.
   */
  int getSourceDimension(neutube::EAxis axis) const;
  int getSourceOffset(neutube::EAxis axis) const;

  /*!
   * \brief Export the bundle into a json file
   */
  void exportJsonFile(const std::string &path) const;

  /*!
   * \brief Get number of layers
   * \return
   */
  int getLayerNumber() const;

  /*!
   * \brief Get the Z coordinate of the start of a layer
   *
   * \a layer must be in [1, layer number].
   */
  double getLayerStart(int layer);

  /*!
   * \brief Get the Z coordinate of the end of a layer
   *
   * \a layer must be in [1, layer number].
   */
  double getLayerEnd(int layer);

  /*!
   * \brief Test if a neuron hits layer
   *
   * \param bodyId ID of the neuron
   * \param top The top layer
   * \param bottom The bottom layer
   * \param isExclusive Exclusively in the range or not
   * \return true iff the skeleton of neuron hits any point between the layer
   *         \a top and the layer \a bottom, and does not hit any other layer
   *         when \a isExclusive is true.
   */
  bool hitsLayer(int bodyId, int top, int bottom, bool isExclusive);

  bool hitsLayer(const ZFlyEmNeuron &neuron, int top, int bottom,
                 bool isExclusive);

  /*!
   * \brief Get the ID map of the class set
   *
   * Each class is assigned to a unique integer number.
   */
  std::map<std::string, int> getClassIdMap() const;

  /*!
   * \brief Set volume entries based on a directory
   *
   * The volume entry is set even the corresponding body file does not exist.
   *
   * \param volumeDir The volume directory path.
   */
  void setVolume(const std::string &volumeDir);

  /*!
   * \brief Set thumbnail entries based on a directory
   *
   * The thumbnail entry is set even the corresponding body file does not exist.
   *
   * \param thumbnailDir The thumbnail directory path.
   */
  void setThumbnail(const std::string &thumbnailDir);

  /*!
   * \brief Get the boundbox of the databundle.
   */
  inline const ZSwcTree* getBoundBox() const { return m_boundBox; }

  /*!
   * \brief Test if a bundle has a bound box
   */
  bool hasBoundBox() const;

  /*!
   * \brief Import bound box from a file.
   */
  void importBoundBox(const std::string &filePath);

  void submitSkeletonizeService() const;

  /*!
   * \brief Upload annotations to DVID server
   *
   * Empty annotations will be ignored.
   */
  void uploadAnnotation(const ZDvidTarget &dvidTarget) const;

private:
  void updateSynapseAnnotation();

private:
  ZFlyEmNeuronArray m_neuronArray;
  std::string m_synapseAnnotationFile;
  std::string m_grayScalePath;
  //std::string m_configFile;
  std::string m_neuronColorFile;
  double m_swcResolution[3];
  double m_imageResolution[3];
  int m_sourceOffset[3];
  int m_sourceDimension[3];
  double m_synapseScale;
  std::vector<double> m_layerRatio;
  ZSwcTree *m_boundBox;

  std::string m_source;
  std::map<std::string, double> m_matchThreshold;

  mutable flyem::ZSynapseAnnotationArray *m_synaseAnnotation;
  mutable std::map<int, QColor> *m_colorMap;

  static const char *m_synapseKey;
  static const char *m_grayScaleKey;
  static const char *m_configKey;
  static const char *m_neuronColorKey;
  static const char *m_synapseScaleKey;
  static const char *m_sourceOffsetKey;
  static const char *m_sourceDimensionKey;
  static const char *m_imageResolutionKey;
  static const char *m_neuronKey;
  static const char *m_swcResolutionKey;
  static const char *m_matchThresholdKey;
  static const char *m_layerKey;
  static const char *m_boundBoxKey;
  static const char *m_serverKey;

  const static int m_layerNumber;
  //const static double m_layerRatio[11];
};

#endif // ZFLYEMDATABUNDLE_H
