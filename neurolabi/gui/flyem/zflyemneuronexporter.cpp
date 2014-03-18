#include "zflyemneuronexporter.h"
#include "zflyemneuronfilter.h"
#include "zvoxel.h"
#include "zobject3dscan.h"

ZFlyEmNeuronExporter::ZFlyEmNeuronExporter() : m_ravelerHeight(0),
  m_usingRavelerCoordinate(false), m_neuronFilter(NULL)
{
}

void ZFlyEmNeuronExporter::exportIdPosition(
    const ZFlyEmNeuronArray &neuronArray, const std::string &filePath)
{
  json_t *rootObj = json_object();
  json_t *neuronArrayObj = json_array();

  for (ZFlyEmNeuronArray::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    bool isSelected = true;
    if (m_neuronFilter != NULL) {
      isSelected = m_neuronFilter->isPassed(neuron);
    }
    if (isSelected) {
      ZObject3dScan *obj = neuron.getBody();
      if (obj != NULL) {
        ZVoxel voxel = obj->getMarker();

        json_t *neuronObj = json_object();
        json_t *idObj = json_integer(neuron.getId());
        json_object_set_new(neuronObj, "id", idObj);


        //voxel.translate(bodyOffset[0], bodyOffset[1], bodyOffset[2]);
        //std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        json_array_append_new(arrayObj, json_integer(voxel.x()));

        int y = voxel.y();
        if (m_usingRavelerCoordinate) {
          y = m_ravelerHeight - 1 - voxel.y();
        }
        json_array_append_new(arrayObj, json_integer(y));
        json_array_append_new(arrayObj, json_integer(voxel.z()));

        json_object_set_new(neuronObj, "position", arrayObj);

        json_array_append_new(neuronArrayObj, neuronObj);
      }
    }
  }

  json_object_set_new(rootObj, "data", neuronArrayObj);
  json_dump_file(rootObj, filePath.c_str(), JSON_INDENT(2));

  json_decref(rootObj);
}

void ZFlyEmNeuronExporter::exportIdVolume(
    const ZFlyEmNeuronArray &neuronArray, const std::string &filePath)
{
  json_t *rootObj = json_object();
  json_t *neuronArrayObj = json_array();

  for (ZFlyEmNeuronArray::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    bool isSelected = true;
    if (m_neuronFilter != NULL) {
      isSelected = m_neuronFilter->isPassed(neuron);
    }
    if (isSelected) {
      ZObject3dScan *obj = neuron.getBody();
      if (obj != NULL) {
        json_t *neuronObj = json_object();
        json_t *idObj = json_integer(neuron.getId());
        json_object_set_new(neuronObj, "id", idObj);

        json_t *volumeObj = json_integer(neuron.getBody()->getVoxelNumber());
        json_object_set_new(neuronObj, "volume", volumeObj);

        json_array_append_new(neuronArrayObj, neuronObj);
      }
      neuron.deprecate(ZFlyEmNeuron::BODY);
    }
  }

  json_object_set_new(rootObj, "data", neuronArrayObj);
  json_dump_file(rootObj, filePath.c_str(), JSON_INDENT(2));

  json_decref(rootObj);
}

