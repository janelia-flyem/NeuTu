#include "zflyemneuronarray.h"
#include "zfilelist.h"
#include "zstring.h"
#include "zobject3dscan.h"
#include "zhdf5writer.h"
#include "misc/miscutility.h"

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

void ZFlyEmNeuronArray::exportBodyToHdf5(const std::string &filePath)
{
  ZHdf5Writer writer;
  writer.open(filePath);

  //writer.createGroup("/");
  writer.createGroup("/bodies");

  for (ZFlyEmNeuronArray::iterator iter = begin(); iter != end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    ZObject3dScan *obj = neuron.getBody();
    if (obj != NULL) {
      if (!obj->isEmpty()) {
#ifdef _DEBUG_
        tic();
#endif
        writer.writeIntArray("/bodies/" + misc::num2str(neuron.getId()) + ".sobj",
                             obj->toIntArray());
#ifdef _DEBUG_
        ptoc();
#endif
      }
    }
    neuron.deprecate(ZFlyEmNeuron::BODY);
  }
}

