#include "zflyemdatabundle.h"

#include <QProcess>
#include <iostream>
#include <sstream>
#include <fstream>
#include "zjsonparser.h"
#include "zstring.h"
#include "tz_error.h"
#include "zgraph.h"
#include "c_json.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zerror.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidfilter.h"
#include "dvid/zdvidwriter.h"
#include "zflyemdvidreader.h"
#include "zintcuboidarray.h"
#include "zfiletype.h"
#include "flyem/zflyemneuroninfo.h"
#include "dvid/zdviddata.h"

using namespace std;

const char *ZFlyEmDataBundle::m_synapseKey = "synapse";
const char *ZFlyEmDataBundle::m_grayScaleKey = "gray_scale";
const char *ZFlyEmDataBundle::m_configKey = "config";
const char *ZFlyEmDataBundle::m_neuronColorKey = "neuron_color";
const char *ZFlyEmDataBundle::m_synapseScaleKey = "synapse_scale";
const char *ZFlyEmDataBundle::m_sourceOffsetKey = "source_offset";
const char *ZFlyEmDataBundle::m_sourceDimensionKey = "source_dimension";
const char *ZFlyEmDataBundle::m_imageResolutionKey = "image_resolution";
const char *ZFlyEmDataBundle::m_neuronKey = "neuron";
const char *ZFlyEmDataBundle::m_swcResolutionKey = "swc_resolution";
const char *ZFlyEmDataBundle::m_matchThresholdKey = "match_threshold";
const char *ZFlyEmDataBundle::m_layerKey = "layer";
const char *ZFlyEmDataBundle::m_boundBoxKey = "bound_box";
const char *ZFlyEmDataBundle::m_serverKey = "dvid";

const int ZFlyEmDataBundle::m_layerNumber = 10;
/*
const double ZFlyEmDataBundle::m_layerRatio[11] = {
  0.0, 0.1, 0.2, 0.3, 0.35, 0.43, 0.54, 0.66, 0.73, 0.91, 1.0};
*/
ZFlyEmDataBundle::ZFlyEmDataBundle() : m_synapseScale(10.0),
  m_boundBox(NULL), m_synaseAnnotation(NULL), m_colorMap(NULL)
{
  for (int k = 0; k < 3; ++k) {
    m_swcResolution[k] = 1.0;
    m_imageResolution[k] = 1.0;
    m_sourceOffset[k] = 0;
    m_sourceDimension[k] = 0;
  }
}

ZFlyEmDataBundle::~ZFlyEmDataBundle()
{
  deprecate(ALL_COMPONENT);
  delete m_boundBox;
}

bool ZFlyEmDataBundle::isDeprecated(EComponent comp) const
{
  switch (comp) {
  case SYNAPSE_ANNOTATION:
    return (m_synaseAnnotation == NULL);
  case COLOR_MAP:
    return (m_colorMap == NULL);
  default:
    break;
  }

  return false;
}

void ZFlyEmDataBundle::deprecate(EComponent comp)
{
  deprecateDependent(comp);

  switch (comp) {
  case SYNAPSE_ANNOTATION:
    delete m_synaseAnnotation;
    m_synaseAnnotation = NULL;
    break;
  case COLOR_MAP:
    delete m_colorMap;
    m_colorMap = NULL;
    break;
  case ALL_COMPONENT:
    deprecate(SYNAPSE_ANNOTATION);
    deprecate(COLOR_MAP);
    break;
  default:
    break;
  }
}

void ZFlyEmDataBundle::deprecateDependent(EComponent comp)
{
  switch (comp) {
  case SYNAPSE_ANNOTATION:
    break;
  default:
    break;
  }
}


bool ZFlyEmDataBundle::loadDvid(const ZDvidFilter &dvidFilter)
{
  ZDvidReader reader;
  reader.open(dvidFilter.getDvidTarget());
  QString info = reader.readInfo("superpixels");

  qDebug() << info;

  const ZDvidTarget &dvidTarget = dvidFilter.getDvidTarget();

  m_source = dvidTarget.getSourceString();

        //"http:" + dvidFilter.getDvidAddress() + ":" +
      //ZString::num2str(dvidFilter.getPort()) + ":" + dvidFilter.getUuid();

  ZDvidInfo dvidInfo;
  dvidInfo.setFromJsonString(info.toStdString());
  dvidInfo.print();

  const ZResolution &voxelResolution = dvidInfo.getVoxelResolution();
  const ZIntPoint &sourceOffset = dvidInfo.getStartCoordinates();
  const std::vector<int> &stackSize = dvidInfo.getStackSize();

  for (int i = 0; i < 3; ++i) {
    m_imageResolution[i] = voxelResolution[i];
    m_swcResolution[i] = voxelResolution[i];
    m_sourceOffset[i] = sourceOffset[i];
    m_sourceDimension[i] = stackSize[i];
  }

  std::set<uint64_t> bodySet;

  if (!dvidFilter.getBodyListFile().empty()) {
    bodySet = dvidFilter.loadBodySet();
  } else {
    if (dvidFilter.hasUpperBodySize()) {
      bodySet = reader.readBodyId(dvidFilter.getMinBodySize(),
                                  dvidFilter.getMaxBodySize());
    } else {
      bodySet = reader.readBodyId(dvidFilter.getMinBodySize());
    }
  }

  m_neuronArray.resize(bodySet.size());
  size_t realSize = 0;

  ZDvidReader fdReader;
  fdReader.open(dvidFilter.getDvidTarget());

//  m_synapseAnnotationFile = dvidTarget.getSourceString();
  QStringList annotationList = fdReader.readKeys(
        ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
                           ZDvidData::ROLE_BODY_LABEL,
                           dvidTarget.getBodyLabelName()).c_str());
  std::set<uint64_t> annotationSet;
  foreach (const QString &idStr, annotationList) {
    annotationSet.insert(ZString(idStr.toStdString()).firstInteger());
  }

  size_t i = 0;
  if (!bodySet.empty()) {
    for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
         iter != bodySet.end(); ++iter, ++i) {
      uint64_t bodyId = *iter;
      if (bodyId > 0 && !dvidFilter.isExcluded(bodyId)) {
        std::string name;
        std::string type;
        if (annotationSet.count(bodyId) > 0) {
          ZFlyEmBodyAnnotation annotation = fdReader.readBodyAnnotation(bodyId);
          name = annotation.getName();

          if (!annotation.getType().empty()) {
            type = annotation.getType();
          } else if (!name.empty()) {
            type = ZFlyEmNeuronInfo::GuessTypeFromName(name);
          }
        }

        bool goodNeuron = true;
        if (dvidFilter.namedBodyOnly() && name.empty()) {
          goodNeuron = false;
        }

        if (goodNeuron) {
          ZFlyEmNeuron &neuron = m_neuronArray[realSize++];
          neuron.setId(bodyId);
          neuron.setName(name);
          neuron.setType(type);
          neuron.setModelPath(m_source);
          neuron.setVolumePath(m_source);
          neuron.setThumbnailPath(m_source);
          neuron.setResolution(m_swcResolution);
          neuron.setSynapseAnnotation(getSynapseAnnotation());
          neuron.setSynapseScale(10 * m_swcResolution[0] + 1);
        }
      }
    }
  } else {
    m_neuronArray.resize(annotationSet.size());
    for (std::set<uint64_t>::const_iterator iter = annotationSet.begin();
         iter != annotationSet.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        ZFlyEmBodyAnnotation annotation = fdReader.readBodyAnnotation(bodyId);
        std::string name = annotation.getName();

        bool goodNeuron = true;
        if (dvidFilter.namedBodyOnly() && name.empty()) {
          goodNeuron = false;
        }

        if (goodNeuron) {
          std::string type;
          if (!annotation.getType().empty()) {
            type = annotation.getType();
          } else if (!name.empty()) {
            type = ZFlyEmNeuronInfo::GuessTypeFromName(name);
          }

          ZFlyEmNeuron &neuron = m_neuronArray[realSize++];
          neuron.setId(bodyId);
          neuron.setName(name);
          neuron.setType(type);
          neuron.setModelPath(m_source);
          neuron.setVolumePath(m_source);
          neuron.setThumbnailPath(m_source);
          neuron.setResolution(m_swcResolution);
          neuron.setSynapseAnnotation(getSynapseAnnotation());
          neuron.setSynapseScale(10 * m_swcResolution[0] + 1);
        }
      }
    }
  }
  m_neuronArray.resize(realSize);
  updateNeuronConnection();

  return true;
}

bool ZFlyEmDataBundle::loadJsonFile(const std::string &filePath)
{
  m_neuronArray.clear();
  deprecate(ALL_COMPONENT);

  if (!fexist(filePath.c_str())) {
    RECORD_WARNING_UNCOND(filePath + " does not exist.");
  }

  m_source = filePath;
  ZJsonObject bundleObject;
  if (bundleObject.load(filePath)) {
    m_synapseAnnotationFile =
        ZJsonParser::stringValue(bundleObject[ZFlyEmDataBundle::m_synapseKey]);
    m_grayScalePath =
        ZJsonParser::stringValue(bundleObject[ZFlyEmDataBundle::m_grayScaleKey]);
    //m_configFile = ZJsonParser::stringValue(bundleObject["config"]);
    m_neuronColorFile =
        ZJsonParser::stringValue(bundleObject[ZFlyEmDataBundle::m_neuronColorKey]);

    json_t *obj = bundleObject[ZFlyEmDataBundle::m_synapseScaleKey];
    if (obj != NULL) {
      m_synapseScale = ZJsonParser::numberValue(obj);
    }

    json_t *sourceOffsetObj = bundleObject[ZFlyEmDataBundle::m_sourceOffsetKey];
    if (sourceOffsetObj != NULL) {
      if (ZJsonParser::isArray(sourceOffsetObj)) {
        for (size_t i = 0; i < 3; ++i) {
          m_sourceOffset[i] = ZJsonParser::integerValue(sourceOffsetObj, i);
        }
      }
    }

    json_t *sourceDimensionObj =
        bundleObject[ZFlyEmDataBundle::m_sourceDimensionKey];
    if (sourceDimensionObj != NULL) {
      if (ZJsonParser::isArray(sourceDimensionObj)) {
        for (size_t i = 0; i < 3; ++i) {
          m_sourceDimension[i] = ZJsonParser::integerValue(sourceDimensionObj, i);
        }
      }
    }

    json_t *imgResObj = bundleObject[ZFlyEmDataBundle::m_imageResolutionKey];
    if (imgResObj != NULL) {
      if (ZJsonParser::isArray(imgResObj)) {
        for (size_t i = 0; i < 3; ++i) {
          m_imageResolution[i] = ZJsonParser::numberValue(imgResObj, i);
        }
      }
    }

    ZJsonArray neuronJsonArray;
    neuronJsonArray.set(bundleObject[ZFlyEmDataBundle::m_neuronKey], false);

    m_neuronArray.resize(neuronJsonArray.size());
    for (size_t i = 0; i < neuronJsonArray.size(); ++i) {
      ZFlyEmNeuron &neuron = m_neuronArray[i];
      ZJsonObject neuronJson(neuronJsonArray.at(i), false);
      neuron.loadJsonObject(neuronJson, getSource());
      neuron.setSynapseAnnotation(getSynapseAnnotation());
      neuron.setSynapseScale(m_synapseScale);
    }



    json_t *swcResObj = bundleObject[ZFlyEmDataBundle::m_swcResolutionKey];
    if (swcResObj != NULL) {
      TZ_ASSERT(ZJsonParser::isArray(swcResObj), "Array object expected.");
      for (size_t i = 0; i < 3; ++i) {
        m_swcResolution[i] = ZJsonParser::numberValue(
              bundleObject[ZFlyEmDataBundle::m_swcResolutionKey], i);
      }
      for (size_t i = 0; i < neuronJsonArray.size(); ++i) {
        m_neuronArray[i].setResolution(m_swcResolution);
      }
    }

    json_t *matchThreObj = bundleObject[ZFlyEmDataBundle::m_matchThresholdKey];
    if (matchThreObj != NULL) {
      FILE *fp = fopen(ZJsonParser::stringValue(matchThreObj), "r");
      if (fp != NULL) {
        ZString line;
        while (line.readLine(fp)) {
          std::string str = line.firstQuotedWord();
          double thre = line.lastDouble();
          m_matchThreshold[str] = thre;
        }
        fclose(fp);
      }
    }
    m_source = filePath;

    ZJsonArray array(bundleObject[ZFlyEmDataBundle::m_layerKey], false);
    m_layerRatio = array.toNumberArray();

    json_t *boundBoxObj = bundleObject[ZFlyEmDataBundle::m_boundBoxKey];
    if (boundBoxObj != NULL) {
      ZString path = ZJsonParser::stringValue(boundBoxObj);
      if (!path.isAbsolutePath()) {
        path = ZString::absolutePath(ZString(m_source).dirPath(), path);
      }
      importBoundBox(path);
    }

    json_t *serverObj = bundleObject[ZFlyEmDataBundle::m_serverKey];
    if (serverObj != NULL) {
      ZJsonObject obj(serverObj, false);
      ZDvidTarget target;
      target.set(ZJsonParser::stringValue(obj["address"]),
          ZJsonParser::stringValue(obj["uuid"]),
          ZJsonParser::integerValue(obj["port"]));
      ZFlyEmDvidReader reader;
      if (reader.open(target)) {
        for (ZFlyEmNeuronArray::iterator iter = m_neuronArray.begin();
             iter != m_neuronArray.end(); ++iter) {
          ZFlyEmNeuron &neuron = *iter;
          ZFlyEmBodyAnnotation annotation = reader.readAnnotation(neuron.getId());
          if (!annotation.getName().empty()) {
            neuron.setName(annotation.getName());
          }
          if (!annotation.getType().empty()) {
            neuron.setType(annotation.getType());
          }
        }
      }
    }

    updateNeuronConnection();

    return true;
  }

  return false;
}

string ZFlyEmDataBundle::toSummaryString() const
{
  ostringstream stream;
  stream << "Source: " << m_source << endl;
  stream << m_neuronArray.size() << " Neurons" << endl;
  stream << "Synapse: " << m_synapseAnnotationFile << endl;
  stream << "Gray scale: " << m_grayScalePath << endl;
  //stream << "Config: " << m_configFile << endl;
  stream << "SWC resolution: " << m_swcResolution[0] << " x "
         << m_swcResolution[1] << " x " << m_swcResolution[2] << endl;

  return stream.str();
}

int ZFlyEmDataBundle::countClass() const
{
  std::set<std::string> classSet;
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = getNeuronArray().begin();
       iter != getNeuronArray().end(); ++iter) {
    if (!iter->getType().empty()) {
      classSet.insert(iter->getType());
    }
  }

  return classSet.size();
}

int ZFlyEmDataBundle::countNeuronByType(const string &className) const
{
  int count = 0;

  for (std::vector<ZFlyEmNeuron>::const_iterator iter = getNeuronArray().begin();
       iter != getNeuronArray().end(); ++iter) {
    if (className == "?") {
      if (iter->getType().empty()) {
        ++count;
      }
    } else {
      if (iter->getType() == className) {
        ++count;
      }
    }
  }

  return count;
}

string ZFlyEmDataBundle::toDetailString() const
{
  ostringstream stream;
  //stream << "Source: " << m_source << endl;
  //stream << m_neuronArray.size() << " neurons" << endl;
  stream << "Details: " << endl;
  stream << "  " << countClass() << " types" << endl;
  stream << "  " << countNeuronByType("?") << " unknown types"
         << endl;
  if (getSynapseAnnotation() != NULL) {
    stream << "  " << getSynapseAnnotation()->getTBarNumber() << " TBars" << endl;
    stream << "  " << getSynapseAnnotation()->getPsdNumber() << " PSDs" << endl;
  }

  /*
  int count[4] = {0, 0, 0, 0};

  const vector<ZFlyEmNeuron> neuronArray = getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    double length = neuron.getModel()->length();
    if (length >= 1000000.0) {
      count[0]++;
    } else if (length >= 500000.0) {
      count[1]++;
    } else if (length >= 100000.0) {
      count[2]++;
    } else {
      count[3]++;
    }
  }

  stream << "Length distribution: " << endl;
  stream << "  >=1000000: " << count[0] << endl;
  stream << "  >= 500000: " << count[1] << endl;
  stream << "  >= 100000: " << count[2] << endl;
  stream << "  <  100000: " << count[3] << endl;
*/

  return stream.str();
}

void ZFlyEmDataBundle::print() const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    iter->print();
  }

  cout << "Synapse: " << m_synapseAnnotationFile << endl;
  cout << "Gray scale: " << m_grayScalePath << endl;
  //cout << "Config: " << m_configFile << endl;
}

string ZFlyEmDataBundle::getModelPath(int bodyId) const
{
  string modelPath;

  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getId() == bodyId) {
      modelPath = iter->getModelPath();
    }
  }

  return modelPath;
}

string ZFlyEmDataBundle::getName(int bodyId) const
{
  string name;

  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getId() == bodyId) {
      name = iter->getName();
    }
  }

  return name;
}

int ZFlyEmDataBundle::getIdFromName(const string &name) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getName() == name) {
      return iter->getId();
    }
  }

  return -1;
}

bool ZFlyEmDataBundle::hasNeuronName(const string &name) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getName() == name) {
      return true;
    }
  }

  return false;
}
const ZFlyEmNeuron* ZFlyEmDataBundle::getNeuron(int bodyId) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getId() == bodyId) {
      return &(*iter);
    }
  }

  return NULL;
}

ZFlyEmNeuron* ZFlyEmDataBundle::getNeuron(int bodyId)
{
  return const_cast<ZFlyEmNeuron*>(
        static_cast<const ZFlyEmDataBundle&>(*this).getNeuron(bodyId));
}

const ZFlyEmNeuron* ZFlyEmDataBundle::getNeuronFromName(const string &name) const
{
  for (vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    if (iter->getName() == name) {
      return &(*iter);
    }
  }

  return NULL;
}

ZSwcTree* ZFlyEmDataBundle::getModel(int bodyId) const
{
  const ZFlyEmNeuron *neuron = getNeuron(bodyId);
  if (neuron == NULL) {
    return NULL;
  }

  return neuron->getModel();
}

void ZFlyEmDataBundle::importSynpaseAnnotation(const string &filePath)
{
  deprecate(SYNAPSE_ANNOTATION);
  m_synapseAnnotationFile = filePath;
  updateSynapseAnnotation();
}

FlyEm::ZSynapseAnnotationArray* ZFlyEmDataBundle::getSynapseAnnotation() const
{
  if (isDeprecated(SYNAPSE_ANNOTATION)) {
    ZString path = m_synapseAnnotationFile;
    if (!path.startsWith("http:") && !path.empty()) {
      if (!path.isAbsolutePath()) {
        path = ZString::absolutePath(ZString(m_source).dirPath(), path);
      }
    }

    if (fexist(path.c_str()) || ZString(path).startsWith("http:")) {
      m_synaseAnnotation = new FlyEm::ZSynapseAnnotationArray;
      m_synaseAnnotation->loadJson(path);
      m_synaseAnnotation->setResolution(m_imageResolution);
      m_synaseAnnotation->setSourceOffset(m_sourceOffset);
      m_synaseAnnotation->setSourceDimension(m_sourceDimension);
    }
  }

  return m_synaseAnnotation;
}

map<int, QColor>* ZFlyEmDataBundle::getColorMap() const
{
  if (isDeprecated(COLOR_MAP)) {
    m_colorMap = new map<int, QColor>;
    ZString colorStr;
    FILE *fp = fopen(m_neuronColorFile.c_str(), "r");
    if (fp != NULL) {
      while (colorStr.readLine(fp)) {
        vector<int> value = colorStr.toIntegerArray();
        if (value.size() >= 4) {
          QColor color;
          color.setRed(value[1]);
          color.setGreen(value[2]);
          color.setBlue(value[3]);
          if (value.size() > 4) {
            color.setAlpha(value[4]);
          }
          (*m_colorMap)[value[0]] = color;
        }
      }
      fclose(fp);
    }
  }

  return m_colorMap;
}

void ZFlyEmDataBundle::updateNeuronConnection()
{
  for (std::vector<ZFlyEmNeuron>::iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    iter->clearConnection();
  }

  if (getSynapseAnnotation() != NULL) {
    ZGraph *graph = getSynapseAnnotation()->getConnectionGraph();
    for (int i = 0; i < graph->getEdgeNumber(); ++i) {
      int inputId = graph->getEdgeBegin(i);
      int outputId = graph->getEdgeEnd(i);
      double weight = graph->getEdgeWeight(i);
      ZFlyEmNeuron *inputNeuron = getNeuron(inputId);
      if (inputNeuron != NULL) {
        ZFlyEmNeuron *outputNeuron = getNeuron(outputId);
        if (outputNeuron != NULL) {
          inputNeuron->appendOutputNeuron(outputNeuron, weight);
          outputNeuron->appendInputNeuron(inputNeuron, weight);
        }
      }
    }
  }
}

double ZFlyEmDataBundle::getImageResolution(NeuTube::EAxis axis)
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return m_imageResolution[0];
  case NeuTube::Y_AXIS:
    return m_imageResolution[1];
  case NeuTube::Z_AXIS:
    return m_imageResolution[2];
  }

  return 1.0;
}

double ZFlyEmDataBundle::getSwcResolution(NeuTube::EAxis axis)
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return m_swcResolution[0];
  case NeuTube::Y_AXIS:
    return m_swcResolution[1];
  case NeuTube::Z_AXIS:
    return m_swcResolution[2];
  }

  return 1.0;
}

int ZFlyEmDataBundle::getSourceDimension(NeuTube::EAxis axis) const
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return m_sourceDimension[0];
  case NeuTube::Y_AXIS:
    return m_sourceDimension[1];
  case NeuTube::Z_AXIS:
    return m_sourceDimension[2];
  }

  return 0;
}

int ZFlyEmDataBundle::getSourceOffset(NeuTube::EAxis axis) const
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return m_sourceOffset[0];
  case NeuTube::Y_AXIS:
    return m_sourceOffset[1];
  case NeuTube::Z_AXIS:
    return m_sourceOffset[2];
  }

  return 0;
}

void ZFlyEmDataBundle::exportJsonFile(const string &path) const
{
  ZJsonObject jsonObj(C_Json::makeObject(), true);

  json_t *neuronArray = C_Json::makeArray();
  ZJsonArray neuronArrayWrapper(neuronArray, false);

  string exportDir = ZString::dirPath(path);

  //Add each neuron
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    ZJsonObject obj = neuron.makeJsonObject(exportDir);
    neuronArrayWrapper.append(obj);
  }

#ifdef _DEBUG_2
  neuronArrayWrapper.print();
#endif

  jsonObj.setEntry(m_neuronKey, neuronArray);

  //Set meta data
  if (!m_synapseAnnotationFile.empty()) {
    jsonObj.setEntry(m_synapseKey, m_synapseAnnotationFile);
  }

  if (!m_grayScalePath.empty()) {
    jsonObj.setEntry(m_grayScaleKey, m_grayScalePath);
  }

  if (!m_neuronColorFile.empty()) {
    jsonObj.setEntry(m_neuronColorKey, m_neuronColorFile);
  }

  jsonObj.setEntry(m_synapseScaleKey, m_synapseScale);
  jsonObj.setEntry(m_sourceDimensionKey, m_sourceDimension, 3);
  jsonObj.setEntry(m_imageResolutionKey, m_imageResolution, 3);
  jsonObj.setEntry(m_swcResolutionKey, m_swcResolution, 3);
  jsonObj.setEntry(m_sourceOffsetKey, m_sourceOffset, 3);

#ifdef _DEBUG_2
  jsonObj.print();
#endif

  jsonObj.dump(path);
}

int ZFlyEmDataBundle::getLayerNumber() const
{
  return m_layerRatio.size() - 1;
}

double ZFlyEmDataBundle::getLayerStart(int layer)
{
  if (layer >= 1 && layer <= getLayerNumber()) {
    return m_layerRatio[layer - 1] * m_sourceDimension[2] * m_imageResolution[2];
  }

  return 0.0;
}

double ZFlyEmDataBundle::getLayerEnd(int layer)
{
  if (layer >= 1 && layer <= getLayerNumber()) {
    return m_layerRatio[layer] * m_sourceDimension[2] * m_imageResolution[2];
  }

  return 0.0;
}

bool ZFlyEmDataBundle::hitsLayer(
    int bodyId, int top, int bottom, bool isExclusive)
{
  ZFlyEmNeuron *neuron = getNeuron(bodyId);
  if (neuron != NULL) {
    return hitsLayer(*neuron, top, bottom, isExclusive);
  }

  return false;
}

bool ZFlyEmDataBundle::hitsLayer(
    const ZFlyEmNeuron &neuron, int top, int bottom, bool isExclusive)
{
  ZSwcTree *tree = neuron.getModel();
  if (tree != NULL) {
    if (top == 0 && bottom == m_layerNumber) {
      return true;
    }
    double topZ = getLayerStart(top);
    double bottomZ = getLayerEnd(bottom);
    const std::vector<Swc_Tree_Node*>& nodeArray = tree->getSwcTreeNodeArray();
    for (std::vector<Swc_Tree_Node*>::const_iterator iter = nodeArray.begin();
         iter != nodeArray.end(); ++iter) {
      if (SwcTreeNode::isRegular(*iter)) {
        if (SwcTreeNode::z(*iter) >= topZ && SwcTreeNode::z(*iter) <= bottomZ) {
          if (!isExclusive) {
            return true;
          }
        }
        if (isExclusive) {
          if (SwcTreeNode::z(*iter) < topZ || SwcTreeNode::z(*iter) > bottomZ) {
            return false;
          }
        }
      }
    }
    if (isExclusive) {
      return true;
    }
  }

  return false;
}

std::map<string, int> ZFlyEmDataBundle::getClassIdMap() const
{
  std::map<string, int> classMap;
  const std::vector<ZFlyEmNeuron> &neuronArray = getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (classMap.count(iter->getType()) == 0) {
      int newClassId = classMap.size() + 1;
      classMap[iter->getType()] = newClassId;
    }
  }

  return classMap;
}

void ZFlyEmDataBundle::setVolume(const string &volumeDir)
{
  std::vector<ZFlyEmNeuron> &neuronArray = getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    ZString volumePath = volumeDir + "/";
    volumePath.appendNumber(neuron.getId());
    volumePath.append(".sobj");
    neuron.setVolumePath(volumePath);
  }
}

void ZFlyEmDataBundle::setThumbnail(const std::string &thumbnailDir)
{
  std::vector<ZFlyEmNeuron> &neuronArray = getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    ZString thumbnailPath = thumbnailDir + "/";
    thumbnailPath.appendNumber(neuron.getId());
    thumbnailPath.append(".tif");
    neuron.setThumbnailPath(thumbnailPath);
  }
}

void ZFlyEmDataBundle::submitSkeletonizeService() const
{
  const QString sklServer = "emrecon100.janelia.priv:9082/skeletonize";
  const std::vector<ZFlyEmNeuron> &neuronArray = getNeuronArray();
  QProcess process;
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    std::string path = neuron.getModelPath();
    ZDvidTarget target;
    target.setFromSourceString(path);

    if (target.isValid()) {
      /*
      QString command = QString(
            "curl -X POST -H \"Content-Type: application/json\" -d "
            "'{\"dvid-server\": \"%1\", \"uuid\": \"%2\", \"bodies\": [%3]}' "
            "http://%4").arg(target.getAddressWithPort().c_str()).
          arg(target.getUuid().c_str()).arg(neuron.getId()).arg(sklServer);
*/
      QString command = "curl";
      QString data = QString(
            "{\"dvid-server\": \"%1\", \"uuid\": \"%2\", \"bodies\": [%3]}").
          arg(target.getAddressWithPort().c_str()).
          arg(target.getUuid().c_str()).arg(neuron.getId());
      QStringList args;
      args << "-X" << "POST" << "-H" << "Content-Type: application/json"
           << "-d" << data
           << QString("http://%4").arg(sklServer);

      qDebug() << command;
      qDebug() << args;
      process.start(command, args);
      process.waitForFinished(300000);
    }
  }
}

bool ZFlyEmDataBundle::hasBoundBox() const
{
  if (m_boundBox != NULL) {
    return !m_boundBox->isEmpty();
  }

  return false;
}

void ZFlyEmDataBundle::importBoundBox(const string &filePath)
{
  if (m_boundBox != NULL) {
    delete m_boundBox;
    m_boundBox = NULL;
  }

  if (ZFileType::fileType(filePath) == ZFileType::SWC_FILE) {
    m_boundBox = new ZSwcTree;
    m_boundBox->load(filePath);
  } else {
    ZIntCuboidArray blockArray;
    blockArray.loadSubstackList(filePath);
    m_boundBox = blockArray.toSwc();
  }

  m_boundBox->rescale(
        m_swcResolution[0], m_swcResolution[1], m_swcResolution[2]);
}

void ZFlyEmDataBundle::uploadAnnotation(const ZDvidTarget &dvidTarget) const
{
  ZDvidWriter writer;
  if (writer.open(dvidTarget)) {
    //for each neuron, update annotation
    for (ZFlyEmNeuronArray::const_iterator iter = m_neuronArray.begin();
         iter != m_neuronArray.end(); ++iter) {
      const ZFlyEmNeuron &neuron = *iter;
      if (!neuron.getName().empty() || !neuron.getType().empty()) {
        writer.writeAnnotation(neuron);
      }
    }
  }
}

void ZFlyEmDataBundle::updateSynapseAnnotation()
{
  for (ZFlyEmNeuronArray::iterator iter = m_neuronArray.begin();
       iter != m_neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    neuron.setSynapseAnnotation(getSynapseAnnotation());
  }
}
