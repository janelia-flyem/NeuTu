#include "zflyemneuronarray.h"
#include "zfilelist.h"
#include "zstring.h"
#include "zobject3dscan.h"

ZFlyEmNeuronArray::ZFlyEmNeuronArray()
{
}

void ZFlyEmNeuronArray::importBodyDir(const std::string &dirPath)
{
  clear();
  ZFileList fileList;
  fileList.load(dirPath, "sobj");
  for (int i = 0; i < fileList.size(); ++i) {
    int bodyId = ZString::lastInteger(fileList.getFilePath(i));
    ZFlyEmNeuron neuron;
    neuron.setId(bodyId);
    neuron.setVolumePath(fileList.getFilePath(i));
    push_back(neuron);
  }
}

void ZFlyEmNeuronArray::setSynapseAnnotation(
    FlyEm::ZSynapseAnnotationArray *annotation)
{
  for (size_t i = 0; i < size(); ++i) {
    ZFlyEmNeuron &neuron = (*this)[i];
    neuron.setSynapseAnnotation(annotation);
  }
}


