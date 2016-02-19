#ifndef ZFLYEMNEURONARRAY_H
#define ZFLYEMNEURONARRAY_H

#include <vector>
#include <string>
#include "zflyemneuron.h"
#include "flyem/zflyemneuronfilter.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidfilter.h"

class ZFlyEmNeuronArray : public std::vector<ZFlyEmNeuron>
{
public:
  ZFlyEmNeuronArray();

  /*!
   * \brief Import neurons from a directory of bodies (*.sobj)
   *
   * \param dirPath Directory of path
   */
  void importBodyDir(const std::string &dirPath);

  void setSynapseAnnotation(FlyEm::ZSynapseAnnotationArray *annotation);

  void exportIdPosition(const std::string &filePath,
                        const ZFlyEmNeuronFilter &filter);

  void exportBodyToHdf5(const std::string &filePath);

  /*!
   * \brief Import named body from body annotation file exported from raveler
   *
   * It clears old elements before loading.
   */
  void importNamedBody(const std::string &filePath);

  void importFromDataBundle(const std::string &filePath);

  /*!
   * \brief Assign classes to the bodies
   *
   * The classes are stored in \a filePath as JSON format:
   *   "neuron": [
   *     {
   *       "id": <int>,
   *       "class": <str>
   *     },
   *     ...
   *   ]
   */
  void assignClass(const std::string &filePath);

  /*!
   * \brief Assign names to the bodies
   *
   * The classes are stored in \a filePath as JSON format:
   *   "data": [
   *     {
   *       "body ID": <int>,
   *       "name": <str>
   *     },
   *     ...
   *   ]
   */
  void assignName(const std::string &filePath);
};

#endif // ZFLYEMNEURONARRAY_H
