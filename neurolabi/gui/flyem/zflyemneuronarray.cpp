#include "zflyemneuronarray.h"
#include "zfilelist.h"
#include "zstring.h"
#include "zobject3dscan.h"
#include "zhdf5writer.h"
#include "zhdf5reader.h"
#include "misc/miscutility.h"

ZFlyEmNeuronArray::ZFlyEmNeuronArray()
{
}

void ZFlyEmNeuronArray::importBodyDir(const std::string &dirPath)
{
  clear();

  std::vector<std::string> pathArray = misc::parseHdf5Path(dirPath);
  if (!pathArray.empty()) {
    ZHdf5Reader reader;
    reader.open(pathArray[0]);
    if (pathArray.size() == 1) {
      pathArray.push_back("/");
    }
    std::vector<std::string> bodyNameArray =
        reader.getAllDatasetName(pathArray[1]);
    for (size_t i = 0; i < bodyNameArray.size(); ++i) {
      if (ZString(bodyNameArray[i]).endsWith(".sobj")) {
        int bodyId = ZString::lastInteger(bodyNameArray[i]);
        ZFlyEmNeuron neuron;
        neuron.setId(bodyId);
        if (!ZString(pathArray[1]).endsWith("/")) {
          pathArray[1] += "/";
        }
        neuron.setVolumePath(pathArray[0] + ":" + pathArray[1] + bodyNameArray[i]);
        push_back(neuron);
      }
    }
  } else {
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
        writer.writeIntArray("/bodies/" + ZString::num2str(neuron.getId()) + ".sobj",
                             obj->toIntArray());
#ifdef _DEBUG_
        ptoc();
#endif
      }
    }
    neuron.deprecate(ZFlyEmNeuron::BODY);
  }
}


void ZFlyEmNeuronArray::importNamedBody(const std::string &filePath)
{
  clear();

  ZJsonObject obj;
  obj.load(filePath);
  if (!obj.isEmpty()) {
    ZJsonArray array(obj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < array.size(); ++i) {
      ZJsonObject bodyObj(array.at(i), false);
      if (bodyObj.hasKey("body ID")) {
        int bodyId = ZJsonParser::integerValue(bodyObj["body ID"]);
        if (bodyId > 0) {
          ZFlyEmNeuron neuron;
          neuron.setId(bodyId);
          if (bodyObj.hasKey("name")) {
            neuron.setName(ZJsonParser::stringValue(bodyObj["name"]));
            push_back(neuron);
          }
        }
      }
    }
  }
}

void ZFlyEmNeuronArray::importFromDataBundle(const std::string &filePath)
{
  clear();

  ZJsonObject obj;
  obj.load(filePath);
  if (!obj.isEmpty()) {
    ZJsonArray array(obj["neuron"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < array.size(); ++i) {
      ZJsonObject bodyObj(array.at(i), false);
      if (bodyObj.hasKey("id")) {
        int bodyId = ZJsonParser::integerValue(bodyObj["id"]);
        if (bodyId > 0) {
          ZFlyEmNeuron neuron;
          neuron.setId(bodyId);
          if (bodyObj.hasKey("class")) {
            neuron.setType(ZJsonParser::stringValue(bodyObj["class"]));
          }
          push_back(neuron);
        }
      }
    }
  }
}

void ZFlyEmNeuronArray::assignClass(const std::string &filePath)
{
  ZJsonObject obj;
  obj.load(filePath);

  if (!obj.isEmpty()) {
    ZJsonArray array(obj["neuron"], ZJsonValue::SET_INCREASE_REF_COUNT);
    if (!array.isEmpty()) {
      std::map<int, std::string> idClassMap;
      for (size_t i = 0; i < array.size(); ++i) {
        ZJsonObject neuronObj(
              array.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
        if (neuronObj.hasKey("id") && neuronObj.hasKey("class")) {
          int bodyId = ZJsonParser::integerValue(neuronObj["id"]);
          std::string type = ZJsonParser::stringValue(neuronObj["class"]);
          idClassMap[bodyId] = type;
        }
      }

      if (!idClassMap.empty()) {
        for (ZFlyEmNeuronArray::iterator iter = begin(); iter != end(); ++iter) {
          ZFlyEmNeuron &neuron = *iter;
          if (idClassMap.count(neuron.getId()) > 0) {
            neuron.setType(idClassMap[neuron.getId()]);
          }
        }
      }
    }
  }
}

void ZFlyEmNeuronArray::assignName(const std::string &filePath)
{
  ZJsonObject obj;
  obj.load(filePath);

  if (!obj.isEmpty()) {
    ZJsonArray array(obj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);
    if (!array.isEmpty()) {
      std::map<int, std::string> idNameMap;
      for (size_t i = 0; i < array.size(); ++i) {
        ZJsonObject neuronObj(
              array.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
        if (neuronObj.hasKey("body ID") && neuronObj.hasKey("name")) {
          int bodyId = ZJsonParser::integerValue(neuronObj["body ID"]);
          std::string name = ZJsonParser::stringValue(neuronObj["name"]);
          idNameMap[bodyId] = name;
        }
      }

      if (!idNameMap.empty()) {
        for (ZFlyEmNeuronArray::iterator iter = begin(); iter != end(); ++iter) {
          ZFlyEmNeuron &neuron = *iter;
          if (idNameMap.count(neuron.getId()) > 0) {
            neuron.setName(idNameMap[neuron.getId()]);
          }
        }
      }
    }
  }
}
