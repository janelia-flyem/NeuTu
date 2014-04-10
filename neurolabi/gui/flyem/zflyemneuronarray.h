#ifndef ZFLYEMNEURONARRAY_H
#define ZFLYEMNEURONARRAY_H

#include <vector>
#include <string>
#include "zflyemneuron.h"
#include "flyem/zflyemneuronfilter.h"

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
};

#endif // ZFLYEMNEURONARRAY_H
