#include "zflyemneuron.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include "zjsonparser.h"
#include "zstring.h"
#include "zswctree.h"
#include "zobject3dscan.h"
#include "swctreenode.h"
#include "c_json.h"
#include "tz_error.h"
#include "zhdf5reader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidurl.h"
#include "flyem/zflyemneuroninfo.h"
#include "neutubeconfig.h"
#include "tz_geo3d_utils.h"
#include "zstackskeletonizer.h"

#if defined(_QT_GUI_USED_)
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "flyem/zskeletonizeservice.h"
#endif

using namespace std;

const int ZFlyEmNeuron::TopMatchCapacity = 10;

#define CONSTRUCTOR_INIT m_sourceId(0), m_id(0), m_synapseScale(10.0), \
    m_model(NULL), m_unscaledModel(NULL), m_buddyModel(NULL), m_body(NULL), \
    m_synapseAnnotation(NULL), m_bodyVolume(0)

const char *ZFlyEmNeuron::m_idKey = "id";
const char *ZFlyEmNeuron::m_nameKey = "name";
const char *ZFlyEmNeuron::m_typeKey = "type";
const char *ZFlyEmNeuron::m_classKey = "class";
const char *ZFlyEmNeuron::m_superclassKey = "superclass";
const char *ZFlyEmNeuron::m_modelKey = "model";
const char *ZFlyEmNeuron::m_volumeKey = "volume";
const char *ZFlyEmNeuron::m_thumbnailKey = "thumbnail";

ZFlyEmNeuron::ZFlyEmNeuron() : CONSTRUCTOR_INIT
{
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = 1.0;
  }
}

ZFlyEmNeuron::ZFlyEmNeuron(int id, ZSwcTree *model, ZObject3dScan *body) :
  CONSTRUCTOR_INIT
{
  m_id = id;
  m_model = model;
  if (model != NULL) {
    m_unscaledModel = model->clone();
  }
  m_body = body;
}

ZFlyEmNeuron::ZFlyEmNeuron(const ZFlyEmNeuron &neuron) : CONSTRUCTOR_INIT
{
  m_sourceId = neuron.m_sourceId;
  m_id = neuron.m_id;
  m_name = neuron.m_name;
  m_type = neuron.m_type;
  m_modelPath = neuron.m_modelPath;
  m_volumePath = neuron.m_volumePath;
  m_thumbnailPath = neuron.m_thumbnailPath;
  m_synapseScale = neuron.m_synapseScale;
  m_synapseAnnotation = neuron.m_synapseAnnotation;
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = neuron.m_resolution[i];
  }
}

ZFlyEmNeuron::~ZFlyEmNeuron()
{
  deprecate(ALL_COMPONENT);
}

bool ZFlyEmNeuron::isDeprecated(EComponent comp) const
{
  switch (comp) {
  case MODEL:
    return (m_model == NULL);
  case BODY:
    return (m_body == NULL);
  case BUDDY_MODEL:
    return (m_buddyModel == NULL);
  case UNSCALED_MODEL:
    return (m_unscaledModel == NULL);
  default:
    break;
  }

  return false;
}

void ZFlyEmNeuron::deprecate(EComponent comp) const
{
  deprecateDependent(comp);

  switch (comp) {
  case MODEL:
    delete m_model;
    m_model = NULL;
    break;
  case UNSCALED_MODEL:
    delete m_unscaledModel;
    m_unscaledModel = NULL;
    break;
  case BODY:
    delete m_body;
    m_body = NULL;
    break;
  case BUDDY_MODEL:
    delete m_buddyModel;
    m_buddyModel = NULL;
    break;
  case ALL_COMPONENT:
    deprecate(MODEL);
    deprecate(BODY);
    deprecate(BUDDY_MODEL);
    break;
  default:
    break;
  }
}

void ZFlyEmNeuron::deprecateDependent(EComponent comp) const
{
  switch (comp) {
  case MODEL:
    deprecate(BUDDY_MODEL);
    break;
  case UNSCALED_MODEL:
    deprecate(MODEL);
    break;
  default:
    break;
  }
}

void ZFlyEmNeuron::setPath(const ZDvidTarget &target)
{
  ZDvidUrl dvidUrl(target);
  m_modelPath = dvidUrl.getSkeletonUrl(getId(), target.getBodyLabelName());
  m_volumePath = dvidUrl.getSparsevolUrl(getId(), target.getBodyLabelName());
  m_thumbnailPath = dvidUrl.getThumbnailUrl(
        getId(), target.getBodyLabelName());
}

ZSwcTree* ZFlyEmNeuron::getResampleBuddyModel(double rs) const
{
  if (isDeprecated(BUDDY_MODEL)) {
    ZSwcTree *model = getModel();
    if (model != NULL) {
      m_buddyModel = model->clone();
      m_buddyModel->resample(rs);
      m_buddyModel->setLabel(0);
    }
  }

  return m_buddyModel;
}

ZSwcTree* ZFlyEmNeuron::getResampleBuddyModel() const
{
  if (isDeprecated(BUDDY_MODEL)) {
    return NULL;
  }

  return m_buddyModel;
}

ZSwcTree* ZFlyEmNeuron::getUnscaledModel(const string &bundleSource) const
{
  if (isDeprecated(UNSCALED_MODEL)) {
    if (!m_modelPath.empty()) {
      ZString path(m_modelPath);
      if (path.startsWith("http:")) {
        updateDvidModel(false);
      } else {
        if (!path.isAbsolutePath()) {
          path = path.absolutePath(ZString::dirPath(bundleSource));
        }
        m_unscaledModel = new ZSwcTree;
        m_unscaledModel->load(path.c_str());
        if (m_unscaledModel->isEmpty()) {
          delete m_unscaledModel;
          m_unscaledModel = NULL;
        }
      }
    }
  }

  return m_unscaledModel;
}


void ZFlyEmNeuron::updateDvidModel(bool forceUpdate) const
{
  if (forceUpdate) {
    deprecate(UNSCALED_MODEL);
  }

  if (isDeprecated(UNSCALED_MODEL)) {
    if (!m_modelPath.empty()) {
      //m_model = new ZSwcTree;
      ZString path(m_modelPath);
#if defined(_QT_GUI_USED_)
      ZDvidReader reader;
      if (path.startsWith("http:") && reader.open(m_modelPath.c_str())) {
        if (!forceUpdate) {
          m_unscaledModel = reader.readSwc(getId());
          if (m_unscaledModel != NULL) {
            if (m_unscaledModel->isEmpty()) {
              delete m_unscaledModel;
              m_unscaledModel = NULL;
            }
          }
        }

        if (m_unscaledModel == NULL) {
          ZStackSkeletonizer skeletonizer;

          ZDvidUrl dvidUrl(reader.getDvidTarget());
          ZJsonObject config = reader.readJsonObject(
                dvidUrl.getSkeletonConfigUrl(
                  reader.getDvidTarget().getBodyLabelName()));

          if (config.isEmpty()) {
            config.load(NeutubeConfig::getInstance().getApplicatinDir() +
                        "/json/skeletonize_fib25_len40.json");
          }
          skeletonizer.configure(config);
          ZObject3dScan obj = reader.readBody(getId());
          if (!obj.isEmpty()) {
            m_unscaledModel = skeletonizer.makeSkeleton(obj);
            if (m_unscaledModel != NULL) {
              ZDvidWriter writer;
              if (writer.open(m_modelPath.c_str())) {
                writer.writeSwc(getId(), m_unscaledModel);
              }
            }
          }
        }
      }
#endif
    }
  }

//  return m_model;
}



ZSwcTree* ZFlyEmNeuron::getModel(const string &bundleSource) const
{
  if (isDeprecated(MODEL)) {
    ZSwcTree *unscaledModel = getUnscaledModel(bundleSource);
    if (unscaledModel != NULL) {
      m_model = unscaledModel->clone();
      m_model->rescale(m_resolution[0], m_resolution[1], m_resolution[2]);
    }
  }
#if 0
  if (isDeprecated(MODEL)) {
    if (!m_modelPath.empty()) {
      //m_model = new ZSwcTree;
      ZString path(m_modelPath);
      if (path.startsWith("http:")) {
        updateDvidModel(false);
      } else {
        if (!path.isAbsolutePath()) {
          path = path.absolutePath(ZString::dirPath(bundleSource));
        }
        m_model = new ZSwcTree;
        m_model->load(path.c_str());
      }
      if (m_model != NULL) {
        if (m_model->data() == NULL) {
          delete m_model;
          m_model = NULL;
        }
      }
    }

    if (m_model != NULL) {
      m_model->rescale(m_resolution[0], m_resolution[1], m_resolution[2]);
    }

#ifdef _DEBUG_2
    if (m_model != NULL) {
      double theta, psi;
//      Geo3d_Normal_Orientation(-0.164321, -0.138413, 0.976647, &theta, &psi);
      Geo3d_Normal_Orientation(0.1, 0.0, 0.995, &theta, &psi);
      m_model->rotate(theta, psi, ZPoint(0, 0, 0), true);
    }
#endif
  }
#endif

  return m_model;
}

ZObject3dScan* ZFlyEmNeuron::getBody() const
{
  if (isDeprecated(BODY)) {
    ZString path(m_volumePath);
    if (path.startsWith("http:")) {
#if defined(_QT_GUI_USED_)
      ZDvidReader reader;
      if (reader.open(m_volumePath.c_str())) {
        m_body = new ZObject3dScan;
        *m_body = reader.readBody(getId());

#ifdef _DEBUG_2
        m_body->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif
      }
#endif
    } else {
      if (!m_volumePath.empty()) {
        m_body = new ZObject3dScan;
        m_body->load(m_volumePath);
      }
    }
  }

  return m_body;
}


void ZFlyEmNeuron::setResolution(const double *res)
{
  if (m_resolution[0] != res[0] || m_resolution[1] != res[1] ||
      m_resolution[2] != res[2]) {
    for (int i = 0; i < 3; ++i) {
      m_resolution[i] = res[i];
    }
    deprecate(MODEL);
  }
}

string ZFlyEmNeuron::getAbsolutePath(const ZString &path, const string &source)
{
  if (!path.isAbsolutePath()) {
    return path.absolutePath(ZString::dirPath(source));
  }

  return path;
}

ZJsonObject ZFlyEmNeuron::getAnnotationJson() const
{
  ZJsonObject obj;
  obj.setEntry(m_nameKey, m_name);
  obj.setEntry(m_typeKey, m_type);

  return obj;
}

void ZFlyEmNeuron::loadJsonObject(ZJsonObject &obj, const string &source)
{
  m_id = ZJsonParser::integerValue(obj[m_idKey]);
  m_name = ZJsonParser::stringValue(obj[m_nameKey]);
  m_type = ZJsonParser::stringValue(obj[m_typeKey]);
  //m_type = ZFlyEmNeuronInfo::GuessTypeFromName(m_name);


  m_modelPath = ZJsonParser::stringValue(obj[m_modelKey]);
  if (!m_modelPath.empty()) {
    m_modelPath = getAbsolutePath(m_modelPath, source);
  }

  m_volumePath = ZJsonParser::stringValue(obj[m_volumeKey]);
  if (!m_volumePath.empty()) {
    m_volumePath = getAbsolutePath(m_volumePath, source);
  }

#ifdef _DEBUG_2
  std::cout << m_volumePath << std::endl;
#endif

  m_thumbnailPath = ZJsonParser::stringValue(obj[m_thumbnailKey]);
  if (!m_thumbnailPath.empty()) {
    m_thumbnailPath = getAbsolutePath(m_thumbnailPath, source);
  }

  deprecate(ALL_COMPONENT);
}

ZJsonObject ZFlyEmNeuron::makeJsonObject() const
{
  json_t *obj = C_Json::makeObject();

  ZJsonObject objWrapper(obj, false);

  objWrapper.setEntry(m_idKey, m_id);
  if (!m_name.empty()) {
    objWrapper.setEntry(m_nameKey, m_name);
  }

  if (!m_type.empty()) {
    objWrapper.setEntry(m_typeKey, m_type);
  }

  if (!m_modelPath.empty()) {
    objWrapper.setEntry(m_modelKey, m_modelPath);
  }

  if (!m_volumePath.empty()) {
    objWrapper.setEntry(m_volumeKey, m_volumePath);
  }

  return objWrapper;
}

ZJsonObject ZFlyEmNeuron::makeJsonObject(const std::string &bundleDir) const
{
  json_t *obj = C_Json::makeObject();

  ZJsonObject objWrapper(obj, false);

  objWrapper.setEntry(m_idKey, m_id);
  if (!m_name.empty()) {
    objWrapper.setEntry(m_nameKey, m_name);
  }

  if (!m_type.empty()) {
    objWrapper.setEntry(m_typeKey, m_type);
  }

  if (!m_modelPath.empty()) {
    objWrapper.setEntry(
          m_modelKey, ZString::relativePath(m_modelPath, bundleDir));
  }

  if (!m_volumePath.empty()) {
    objWrapper.setEntry(
          m_volumeKey, ZString::relativePath(m_volumePath, bundleDir));
  }

  if (!m_thumbnailPath.empty()) {
    objWrapper.setEntry(
          m_thumbnailKey, ZString::relativePath(m_thumbnailPath, bundleDir));
  }

  return objWrapper;
}


void ZFlyEmNeuron::print() const
{
  print(cout);
}

void ZFlyEmNeuron::print(ostream &stream) const
{
  stream << "Neuron: " << endl;
  stream << "  Id: " << m_id << endl;
  stream << "  Name: " << m_name << endl;
  stream << "  Type: " << m_type << endl;
  stream << "  Model: " << m_modelPath << endl;
  stream << "  Volume: " << m_volumePath << endl;
  stream << "  Thumbnail: " << m_thumbnailPath << endl;
}

void ZFlyEmNeuron::setId(const string &str)
{
  setId(atoi(str.c_str()));
}

void ZFlyEmNeuron::printJson(ostream &stream, int indent) const
{
  stream << setfill(' ');
  stream << setw(indent) << "";
  stream << "{" << endl;
  stream << setw(indent) << "";
  stream << "  \"id\": " << m_id << "," << endl;
  stream << setw(indent) << "";
  stream << "  \"name\": \"" << m_name << "\"," << endl;
  stream << setw(indent) << "";
  stream << "  \"type\": \"" << m_type << "\"," << endl;
  stream << setw(indent) << "";
  stream << "  \"model\": \"" << m_modelPath << "\"" << endl;
  stream << setw(indent) << "";
  stream << '}' << endl;
}

bool ZFlyEmNeuron::hasSimilarName(const string &name) const
{
  ZString newName = m_name;

  for (ZString::iterator iter = newName.begin(); iter != newName.end();
       ++iter) {
    if ((!tz_isletter(*iter)) && (!isdigit(*iter))) {
      *iter = '_';
    }
  }

  ZString queryName = name;
  for (ZString::iterator iter = queryName.begin(); iter != queryName.end();
       ++iter) {
    if ((!tz_isletter(*iter)) && (!isdigit(*iter))) {
      *iter = '_';
    }
  }

#ifdef _DEBUG_2
  cout << newName << endl;
#endif

  return newName.contains(queryName.c_str());
}

#ifdef _QT_GUI_USED_
std::vector<ZPunctum*> ZFlyEmNeuron::getSynapse() const
{
  /*
  std::vector<ZPunctum*> synapse;
  if (m_synapseAnnotation != NULL) {
    FlyEm::SynapseAnnotationConfig config;
    config.startNumber = m_synapseAnnotation->getMetaData().getSourceZOffset();
    config.height = m_synapseAnnotation->getMetaData().getSourceYDim();
    config.xResolution = m_synapseAnnotation->getMetaData().getXResolution();
    config.yResolution = m_synapseAnnotation->getMetaData().getYResolution();
    config.zResolution = m_synapseAnnotation->getMetaData().getZResolution();
    config.swcDownsample1 = 1;
    config.swcDownsample2 = 1;
    config.sizeScale = m_synapseScale;

    FlyEm::SynapseDisplayConfig displayConfig;
    displayConfig.mode = FlyEm::SynapseDisplayConfig::HALF_SYNAPSE;
    displayConfig.tBarColor.red = 255;
    displayConfig.tBarColor.green = 255;
    displayConfig.tBarColor.blue = 0;
    displayConfig.psdColor.red = 0;
    displayConfig.psdColor.green = 0;
    displayConfig.psdColor.blue = 0;
    displayConfig.bodyId = getId();

    return m_synapseAnnotation->toPuncta(
          config, FlyEm::SynapseLocation::PHYSICAL_SPACE, displayConfig);
  }

  return synapse;
  */

  return getSynapse(-1);
}

std::vector<ZPunctum*> ZFlyEmNeuron::getSynapse(
    int buddyBodyId) const
{
  std::vector<ZPunctum*> synapse;
  if (m_synapseAnnotation != NULL) {
    FlyEm::SynapseAnnotationConfig config;
    config.startNumber = m_synapseAnnotation->getMetaData().getSourceZOffset();
    config.height = m_synapseAnnotation->getMetaData().getSourceYDim();
    config.xResolution = m_synapseAnnotation->getMetaData().getXResolution();
    config.yResolution = m_synapseAnnotation->getMetaData().getYResolution();
    config.zResolution = m_synapseAnnotation->getMetaData().getZResolution();
    config.swcDownsample1 = 1;
    config.swcDownsample2 = 1;
    config.sizeScale = m_synapseScale;

    FlyEm::SynapseDisplayConfig displayConfig;
    displayConfig.mode = FlyEm::SynapseDisplayConfig::HALF_SYNAPSE;
    displayConfig.tBarColor.red = 255;
    displayConfig.tBarColor.green = 255;
    displayConfig.tBarColor.blue = 0;
    displayConfig.psdColor.red = 0;
    displayConfig.psdColor.green = 0;
    displayConfig.psdColor.blue = 0;
    displayConfig.bodyId = getId();
    displayConfig.buddyBodyId = buddyBodyId;

    return m_synapseAnnotation->toPuncta(
          config, FlyEm::SynapseLocation::PHYSICAL_SPACE, displayConfig);
  }

  return synapse;
}
#endif

int ZFlyEmNeuron::getTBarNumber() const
{
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countTBar(getId());
  }

  return 0;
}

int ZFlyEmNeuron::getPsdNumber() const
{
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countPsd(getId());
  }

  return 0;
}

int ZFlyEmNeuron::getInputNeuronNumber() const
{
  /*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countInputNeuron(getId());
  }

  return 0;
  */

  return m_input.size();
}

int ZFlyEmNeuron::getOutputNeuronNumber() const
{
  /*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->countOutputNeuron(getId());
  }

  return 0;
  */

  return m_output.size();
}

const ZFlyEmNeuron *ZFlyEmNeuron::getStrongestInput() const
{
  if (m_input.empty()) {
    return NULL;
  }

  size_t maxIndex = 0;
  double maxWeight = m_inputWeight[0];

  for (size_t i = 1; i < m_input.size(); ++i) {
    if (maxWeight < m_inputWeight[i]) {
      maxIndex = i;
      maxWeight = m_inputWeight[i];
    }
  }

  return m_input[maxIndex];
/*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->getStrongestInput(getId());
  }
  return -1;

  return NULL;
  */
}

const ZFlyEmNeuron* ZFlyEmNeuron::getStrongestOutput() const
{
  if (m_output.empty()) {
    return NULL;
  }

  size_t maxIndex = 0;
  double maxWeight = m_outputWeight[0];

  for (size_t i = 1; i < m_output.size(); ++i) {
    if (maxWeight < m_outputWeight[i]) {
      maxIndex = i;
      maxWeight = m_outputWeight[i];
    }
  }

  return m_output[maxIndex];
  /*
  if (m_synapseAnnotation != NULL) {
    return m_synapseAnnotation->getStrongestOutput(getId());
  }
  return -1;

  return NULL;
  */
}

void ZFlyEmNeuron::clearConnection()
{
  m_input.clear();
  m_output.clear();
  m_inputWeight.clear();
  m_outputWeight.clear();
}

const ZFlyEmNeuron* ZFlyEmNeuron::getInputNeuron(size_t index) const
{
  if (index >= m_input.size()) {
    return NULL;
  }

  return m_input[index];
}

const ZFlyEmNeuron* ZFlyEmNeuron::getOutputNeuron(size_t index) const
{
  if (index >= m_output.size()) {
    return NULL;
  }

  return m_output[index];
}

string ZFlyEmNeuron::toString() const
{
  ostringstream stream;
  stream << m_id << " (" << m_type << ") : " << m_modelPath;

  return stream.str();
}

double ZFlyEmNeuron::getBodyVolume(bool cacheBody) const
{
  if (m_bodyVolume == 0) {
    m_bodyVolume = getBody()->getVoxelNumber();
  }

  if (!cacheBody) {
    deprecate(ZFlyEmNeuron::BODY);
  }

  return m_bodyVolume;
#if 0
  if (m_volumePath.empty()) {
    return 0.0;
  }

  ZObject3dScan obj;
  if (obj.load(m_volumePath)) {
    return obj.getVoxelNumber() * m_resolution[0] * m_resolution[1] *
        m_resolution[2];
  }

  return 0.0;
#endif
}

ZFlyEmNeuronAxis ZFlyEmNeuron::getAxis() const
{
  ZSwcTree *tree = getModel();

  ZFlyEmNeuronAxis axis;

  if (tree != NULL) {
    double dz = m_resolution[2] * 5.0;

    //Get all nodes sorted by z
    const std::vector<Swc_Tree_Node *> &nodeArray = tree->getSwcTreeNodeArray(
          ZSwcTree::Z_SORT_ITERATOR);

    if (!nodeArray.empty()) {
      TZ_ASSERT(SwcTreeNode::isRegular(nodeArray[0]), "Unexpected virtual node");

      //Build axis
      ZPoint center;
      double currentZ = SwcTreeNode::z(nodeArray[0]);
      double weight = 0.0;
      for (size_t i = 0; i < nodeArray.size(); ++i) {
        double z = SwcTreeNode::z(nodeArray[i]);
        if (z >= currentZ - dz && z < currentZ + dz) {
          double decay = (z - currentZ) / dz;
          double v = SwcTreeNode::radius(nodeArray[i]) *
              SwcTreeNode::radius(nodeArray[i]) *
              SwcTreeNode::radius(nodeArray[i]) * exp(-(decay * decay));
          center += SwcTreeNode::center(nodeArray[i]) * v;
          weight += v;
        } else {
          if (weight > 0.0) {
            center /= weight;
            center.setZ(currentZ);
            axis.setCenter(center);
          }
          center.set(0, 0, 0);
          weight = 0.0;
          currentZ += dz * 2.0;
        }
      }
    }
  }

  return axis;
}

ZFlyEmNeuronRange ZFlyEmNeuron::getRange(double xyRes, double zRes) const
{
  ZFlyEmNeuronRange range;

  ZObject3dScan *obj = getBody();


  int minZ = obj->getMinZ();
  int maxZ = obj->getMaxZ();

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice = obj->getSlice(z);

    if (!slice.isEmpty()) {
      ZPoint center = slice.getCentroid();
      double maxDist = 0.0;
      for (size_t i = 0; i < slice.getStripeNumber(); ++i) {
        int nseg = slice.getStripe(i).getSegmentNumber();
        int y = slice.getStripe(i).getY();
        for (int j = 0; j < nseg; ++j) {
          int x1 = slice.getStripe(i).getSegmentStart(j);
          int x2 = slice.getStripe(i).getSegmentEnd(j);
          double dist1 = center.distanceTo(x1, y, z);
          double dist2 = center.distanceTo(x2, y, z);
          maxDist = max(dist1, dist2);
        }
      }

      range.setPlaneRange(z * zRes, maxDist  * xyRes);
    }
  }

  return range;
}

bool ZFlyEmNeuron::hasType() const
{
  return !getType().empty();
}

std::string ZFlyEmNeuron::getClass() const
{
  return ZFlyEmNeuronInfo::GetClassFromType(getType());
}

std::string ZFlyEmNeuron::getSuperclass() const
{
  return ZFlyEmNeuronInfo::GetSuperclassFromType(getType());
}

void ZFlyEmNeuron::releaseBody()
{
  m_body = NULL;
}

void ZFlyEmNeuron::releaseModel()
{
  m_model = NULL;
}

bool ZFlyEmNeuron::importBodyFromHdf5(
    const std::string &filePath, const std::string &key)
{
  deprecate(BODY);
  ZHdf5Reader reader;
  if (reader.open(filePath)) {
    std::vector<int> array = reader.readIntArray(key);
    if (!array.empty()) {
      if (m_body == NULL) {
        m_body = new ZObject3dScan;
      }

      return m_body->load(&(array[0]), array.size());
    }

    return false;
  }

  return false;
}

void ZFlyEmNeuron::setModel(ZSwcTree *tree)
{
  m_model = tree;
}

void ZFlyEmNeuron::setUnscaledModel(ZSwcTree *tree)
{
  m_unscaledModel = tree;
}
