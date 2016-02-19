#ifndef ZFLYEMNEURON_H
#define ZFLYEMNEURON_H

#include <string>
#include <ostream>
#include "zjsonobject.h"
#include "zsynapseannotationarray.h"
#include "zstring.h"
#include "zflyemneuronaxis.h"
#include "zflyemneuronrange.h"


class ZSwcTree;
class ZPunctum;
class ZObject3dScan;
class ZDvidTarget;

/*!
 * \brief The class of Fly EM neuron
 *
 * The returned model is supposed to be with the physical size in the unit 'um'.
 */

class ZFlyEmNeuron
{
public:
  ZFlyEmNeuron();
  ZFlyEmNeuron(int id, ZSwcTree *model, ZObject3dScan *body);
  ~ZFlyEmNeuron();

  ZFlyEmNeuron(const ZFlyEmNeuron &neuron);

  enum EComponent {
    MODEL, UNSCALED_MODEL, BODY, BUDDY_MODEL, ALL_COMPONENT
  };

  bool isDeprecated(EComponent comp) const;
  void deprecate(EComponent comp) const;
  void deprecateDependent(EComponent comp) const;

  void loadJsonObject(ZJsonObject &obj, const std::string &source);
  ZJsonObject getAnnotationJson() const;

  /*!
   * \brief Make a json object from the neuron.
   *
   * The user is responsible to free the object.
   *
   * \return A pointer to the json object.
   */
 ZJsonObject makeJsonObject() const;

  /*!
   * \brief Make a json object from the neuron.
   *
   * The file paths associated with the neuron are converted into relative path.
   */
  ZJsonObject makeJsonObject(const std::string &bundleDir) const;

  inline void setId(int id) {
    m_id = id;
  }

  inline void setName(const std::string &name) {
    m_name = name;
  }

  inline std::string getName() const {
    return m_name;
  }


  inline void setType(const std::string &c) {
    m_type = c;
  }

  inline const std::string& getType() const {
    return m_type;
  }

  std::string getClass() const;
  std::string getSuperclass() const;

  /*!
   * \brief hasClass
   * \return true if the neuron has been assigned to a class
   */
  bool hasType() const;

  void setId(const std::string &str);

  inline int getId() const { return m_id; }
  inline int getSourceId() const { return m_sourceId; }
  inline void setSourceId(int id) { m_sourceId = id; }

  void setPath(const ZDvidTarget &target);

  inline void setModelPath(const std::string &path) {
    m_modelPath = path;
  }

  inline const std::string& getModelPath() const {
    return m_modelPath;
  }

  inline const std::string& getVolumePath() const {
    return m_volumePath;
  }

  inline const std::string& getThumbnailPath() const {
    return m_thumbnailPath;
  }

  void updateDvidModel(bool forceUpdate) const;

  ZSwcTree *getModel(const std::string &bundleSource = "") const;
  ZSwcTree *getUnscaledModel(const std::string &bundleSource = "") const;

  void setModel(ZSwcTree *tree);
  void setUnscaledModel(ZSwcTree *tree);

  /*!
   * \brief Get the buddy model from resampling
   *
   * The function will always return the buffered buddy model no matter where
   * the model is from.
   *
   * \return The buddy model.
   */
  ZSwcTree *getResampleBuddyModel(double rs) const;

  /*!
   * \brief Get the buddy model from resampling
   *
   * The function will always return the buffered buddy model no matter where
   * the model is from and it does not trigger model calculation. It returns
   * NULL if the model is deprecated;
   *
   * \return The buddy model.
   */
  ZSwcTree *getResampleBuddyModel() const;

  /*!
   * \brief Get medical axis of the model along z.
   *
   * It returns an empty axis if no model is retrieved.
   */
  ZFlyEmNeuronAxis getAxis() const;

  void setResolution(const double *res);

  void print() const;
  void print(std::ostream &stream) const;
  void printJson(std::ostream &stream, int indent) const;

  bool hasSimilarName(const std::string &name) const;

  //inline const std::string& getPredictedClass() { return m_predictedClass; }
  inline const std::vector<const ZFlyEmNeuron*>& getTopMatch() const {
    return m_matched;
  }

  template <typename InputIterator>
  void setMatched(const InputIterator &begin, const InputIterator &end) const;

  inline void setSynapseScale(double scale) {
    m_synapseScale = scale;
  }

#ifdef _QT_GUI_USED_
  std::vector<ZPunctum*> getSynapse() const;
  std::vector<ZPunctum*> getSynapse(int buddyBodyId) const;
#endif
  inline void setSynapseAnnotation(FlyEm::ZSynapseAnnotationArray *annotation) {
    m_synapseAnnotation = annotation;
  }

  /*!
   * \brief Get the number of TBars on the neuron
   *
   * It returns 0 if the synapse annotation is not available
   */
  int getTBarNumber() const;

  /*!
   * \brief Get the number of PSDs on the neuron
   *
   * It returns 0 if the synapse annotation is not available
   */
  int getPsdNumber() const;

  int getInputNeuronNumber() const;
  int getOutputNeuronNumber() const;
  const ZFlyEmNeuron* getStrongestInput() const;
  const ZFlyEmNeuron* getStrongestOutput() const;

  inline void appendInputNeuron(const ZFlyEmNeuron *neuron, double weight) {
    m_input.push_back(neuron);
    m_inputWeight.push_back(weight);
  }

  inline void appendOutputNeuron(const ZFlyEmNeuron *neuron, double weight) {
    m_output.push_back(neuron);
    m_outputWeight.push_back(weight);
  }

  void clearConnection();

  const ZFlyEmNeuron* getInputNeuron(size_t index) const;
  const ZFlyEmNeuron* getOutputNeuron(size_t index) const;

  std::string toString() const;

  ZObject3dScan* getBody() const;

  /*!
   * \brief Get the range of the neuron
   */
  ZFlyEmNeuronRange getRange(double xyRes, double zRes) const;

  /*!
   * \brief Compute the volume of the orignal body (before skeletonization)
   * \return Volume in number of voxels. 0 if the volume data is not available.
   */
  double getBodyVolume(bool cacheBody = true) const;

  /*!
   * \brief Set the volume path
   */
  inline void setVolumePath(const std::string &path) {
    m_volumePath = path;
  }

  /*!
   * \brief Set thumbnail path
   */
  inline void setThumbnailPath(const std::string &path) {
    m_thumbnailPath = path;
  }

  inline const double *getSwcResolution() const {
    return m_resolution;
  }

  /*!
   * \brief Import body form an HDF5 file
   *
   * \param filePath Path of the HDF5 file
   * \param key Key of the body
   * \return true iff the body is loaded successfully
   */
  bool importBodyFromHdf5(
      const std::string &filePath, const std::string &key);

  static const int TopMatchCapacity;

  //Interfaces for SWIG. Do not use them in native applications
  void releaseBody();
  void releaseModel();
  /**************************SWIG End***************/



private:
  std::string getAbsolutePath(const ZString &path, const std::string &source);

private:
  int m_sourceId;
  int m_id;
  std::string m_name;
  std::string m_type;
  std::string m_modelPath;
  std::string m_volumePath;
  std::string m_thumbnailPath;
  double m_resolution[3]; //Resolution of the swc file saved as m_modelPath
                          //The swc file and volume file must have the same
                          //resolution
  double m_synapseScale;

  //input and output should be sorted by connection strength
  std::vector<const ZFlyEmNeuron*> m_input;
  std::vector<const ZFlyEmNeuron*> m_output;
  std::vector<double> m_inputWeight;
  std::vector<double> m_outputWeight;

  mutable ZSwcTree *m_model;
  mutable ZSwcTree *m_unscaledModel;
  mutable ZSwcTree *m_buddyModel;
  mutable ZObject3dScan *m_body;
  mutable std::vector<const ZFlyEmNeuron*> m_matched;
  const FlyEm::ZSynapseAnnotationArray *m_synapseAnnotation;
  mutable size_t m_bodyVolume;

  static const char *m_idKey;
  static const char *m_nameKey;
  static const char *m_typeKey;
  static const char *m_classKey;
  static const char *m_superclassKey;
  static const char *m_modelKey;
  static const char *m_volumeKey;
  static const char *m_thumbnailKey;
};

template <typename InputIterator>
void ZFlyEmNeuron::setMatched(
    const InputIterator &begin, const InputIterator &end) const
{
  m_matched.clear();
  int count = 1;
  for (InputIterator iter = begin; iter != end; ++iter, ++count) {
    m_matched.push_back(*iter);
    if (count > TopMatchCapacity) {
      break;
    }
  }
}

#endif // ZFLYEMNEURON_H
