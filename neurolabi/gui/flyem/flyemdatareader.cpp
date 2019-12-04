#include "flyemdatareader.h"

#include <memory>
#include <QByteArray>

//#include "geometry/zintpoint.h"
//#include "geometry/zintcuboid.h"

#include "zstring.h"
#include "zjsondef.h"
#include "neutubeconfig.h"
#include "zmesh.h"
#include "zswctree.h"

#include "zjsonparser.h"
#include "zjsonobjectparser.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zobject3dscan.h"

#include "zmeshfactory.h"

#include "logging/zlog.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidsynapse.h"

#include "zdvidutil.h"

#include "flyemdataconfig.h"
#include "zflyemneuronbodyinfo.h"
#include "zflyembodyannotation.h"
#include "zflyemutilities.h"

FlyEmDataReader::FlyEmDataReader()
{

}

FlyEmDataConfig FlyEmDataReader::ReadDataConfig(const ZDvidReader &reader)
{
  std::string userName = NeutubeConfig::GetUserName();
  ZJsonObject obj = reader.readJsonObject(
        ZDvidUrl(reader.getDvidTarget()).getDataConfigUrl(userName));

  if (!obj.hasKey(FlyEmDataConfig::KEY_CONTRAST)) {
    ZJsonObject contrastJson = reader.readContrastProtocal();
    obj.setEntry(FlyEmDataConfig::KEY_CONTRAST, contrastJson);
  }

  ZJsonObject bodyStatusJson = reader.readBodyStatusV2();
  obj.setEntry(FlyEmDataConfig::KEY_BODY_STATUS, bodyStatusJson);

  FlyEmDataConfig config;
  config.loadJsonObject(obj);

  return config;
}

ZFlyEmNeuronBodyInfo FlyEmDataReader::ReadBodyInfo(
    const ZDvidReader &reader, uint64_t bodyId)
{
  ZJsonObject obj;

  QByteArray byteArray = reader.readKeyValue(
        ZDvidData::GetName(ZDvidData::ERole::BODY_INFO,
                           ZDvidData::ERole::BODY_LABEL,
                           reader.getDvidTarget().getBodyLabelName()).c_str(),
        ZString::num2str(bodyId).c_str());
  if (!byteArray.isEmpty()) {
    obj.decode(byteArray.constData());
  }

  ZFlyEmNeuronBodyInfo bodyInfo;
  bodyInfo.loadJsonObject(obj);

  return bodyInfo;
}

ZFlyEmBodyAnnotation FlyEmDataReader::ReadBodyAnnotation(
    const ZDvidReader &reader, uint64_t bodyId)
{
  ZFlyEmBodyAnnotation annotation;

  if (reader.getDvidTarget().hasBodyLabel()) {
    ZDvidUrl url(reader.getDvidTarget());

    QByteArray data = reader.readBuffer(url.getBodyAnnotationUrl(bodyId));
    annotation.loadJsonString(data.constData());
    annotation.setBodyId(bodyId);
  }

  return annotation;
}

std::vector<ZFlyEmToDoItem> FlyEmDataReader::ReadToDoItem(
    const ZDvidReader &reader, const ZIntCuboid &box)
{
  ZDvidUrl dvidUrl(reader.getDvidTarget());
  ZJsonArray obj = reader.readJsonArray(dvidUrl.getTodoListUrl(box));

  std::vector<ZFlyEmToDoItem> itemArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject itemJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZFlyEmToDoItem &item = itemArray[i];
    item.loadJsonObject(itemJson, dvid::EAnnotationLoadMode::PARTNER_RELJSON);
  }

  return itemArray;
}

ZFlyEmToDoItem FlyEmDataReader::ReadToDoItem(
      const ZDvidReader &reader, int x, int y, int z)
{
  std::vector<ZFlyEmToDoItem> itemArray =
      ReadToDoItem(reader, ZIntCuboid(x, y, z, x, y, z));
  if (!itemArray.empty()) {
    return itemArray[0];
  }

  return ZFlyEmToDoItem();
}

ZMesh* FlyEmDataReader::LoadRoi(
    const ZDvidReader &reader, const std::string &roiName,
    const std::string &key, const std::string &source)
{
  ZMesh *mesh = NULL;

  if (!roiName.empty() && !key.empty()) {

#ifdef _DEBUG_
    std::cout << "Add ROI: " << "from " << key << " (" << source << ")"
              << " as " << roiName << std::endl;
#endif

    if (source == "roi") {
      ZObject3dScan roi;
      reader.readRoi(key, &roi);
      if (!roi.isEmpty()) {
        ZMeshFactory mf;
        mf.setOffsetAdjust(true);
        mesh = mf.makeMesh(roi);
//        mesh = ZMeshFactory::MakeMesh(roi);

        //      m_loadedROIs.push_back(roi);
        //      std::string source =
        //          ZStackObjectSourceFactory::MakeFlyEmRoiSource(roiName);
        //      m_roiSourceList.push_back(source);
      }
    } else if (source == "mesh") {
      mesh = reader.readMesh(
            ZDvidData::GetName(ZDvidData::ERole::ROI_DATA_KEY), key);
    }
  }

  return mesh;
}

ZMesh* FlyEmDataReader::LoadRoi(
    const ZDvidReader &reader, const std::string &roiName,
    const std::vector<std::string> &keyList, const std::string &source)
{
  ZMesh *mesh = NULL;
  if (!roiName.empty() && !keyList.empty()) {
    if (source == "roi") {
      ZObject3dScan roi;
      for (const std::string &key : keyList) {
        reader.readRoi(key, &roi, true);
      }
      if (!roi.isEmpty()) {
        mesh = ZMeshFactory::MakeMesh(roi);
      }
    } else if (source == "mesh") {
      if (keyList.size() == 1) {
        mesh = reader.readMesh(
              ZDvidData::GetName(ZDvidData::ERole::ROI_DATA_KEY), keyList[0]);
      } else {
        std::vector<ZMesh*> meshList;
        for (const std::string &key : keyList) {
          ZMesh *submesh = reader.readMesh(
                ZDvidData::GetName(ZDvidData::ERole::ROI_DATA_KEY), key);
          if (submesh != NULL) {
            meshList.push_back(submesh);
          }
        }
        if (!meshList.empty()) {
          mesh = new ZMesh;
          *mesh = ZMesh::Merge(meshList);
          for (ZMesh *submesh : meshList) {
            delete submesh;
          }
        }
      }
    }
  }

  return mesh;
}

ZMesh* FlyEmDataReader::ReadRoiMesh(
    const ZDvidReader &reader, const std::string &roiName,
    std::function<void(std::string)> errorMsgHandler)
{
  ZMesh *mesh = NULL;

  ZJsonObject roiInfo = reader.readJsonObjectFromKey(
        ZDvidData::GetName(ZDvidData::ERole::ROI_KEY).c_str(), roiName.c_str());
  ZJsonObjectParser parser;
  bool visible = parser.getValue(roiInfo, "visible", true);

  if (visible && roiInfo.hasKey(neutu::json::REF_KEY)) {
    ZJsonObject jsonObj(roiInfo.value(neutu::json::REF_KEY));

    std::string type = ZJsonParser::stringValue(jsonObj["type"]);
    if (type.empty()) {
      type = "mesh";
    }

    if (ZJsonParser::IsArray(jsonObj["key"])) {
      ZJsonArray arrayJson(jsonObj.value("key"));
      std::vector<std::string> keyList;
      for (size_t i = 0; i < arrayJson.size(); ++i) {
        std::string key = ZJsonParser::stringValue(arrayJson.at(i));
        if (!key.empty()) {
          keyList.push_back(key);
        }
      }
      mesh = LoadRoi(reader, roiName, keyList, type);

    } else {
      std::string key = ZJsonParser::stringValue(jsonObj["key"]);
      if (key.empty()) {
        key = roiName;
      }
      mesh = LoadRoi(reader, roiName, key, type);
    }

    if (mesh == nullptr) {
      errorMsgHandler("Failed to load ROI " + roiName);
    }
  }

  return mesh;
}

ZObject3dScan* FlyEmDataReader::ReadRoi(
      const ZDvidReader &reader, const std::vector<std::string> &roiList,
      ZObject3dScan *result)
{
  if (result) {
    result->clear();
  } else {
    result = new ZObject3dScan;
  }

  for (const std::string roi : roiList) {
    reader.readRoi(roi, result, true);
  }

  return result;
}

ZDvidSparseStack* FlyEmDataReader::ReadDvidSparseStack(
    const ZDvidTarget &target, ZDvidReader *grayscaleReader,
    uint64_t bodyId, neutu::EBodyLabelType labelType, bool async)
{
  ZDvidSparseStack *spStack = nullptr;
  if (target.isValid() && target.hasSegmentation()) {
    spStack = new ZDvidSparseStack;
    spStack->setLabelType(labelType);
    spStack->setLabel(bodyId);
    if (grayscaleReader) {
      if (grayscaleReader->isReady()) {
        spStack->setGrayscaleReader(*grayscaleReader); //Need to set before target
      }
    }
    spStack->setDvidTarget(target);
    if (async) {
      spStack->loadBodyAsync(bodyId);
    } else {
      spStack->loadBody(bodyId);
    }
  }

  return spStack;
}

std::unordered_map<ZIntPoint, uint64_t> FlyEmDataReader::ReadSynapseLabel(
      const ZDvidReader &reader, const std::vector<ZDvidSynapse>& synapseArray)
{
  std::unordered_map<ZIntPoint, uint64_t> result;
  for (const ZDvidSynapse &synapse : synapseArray) {
    if (synapse.getBodyId() > 0) {
      result[synapse.getPosition()] = synapse.getBodyId();
    }
  }

  std::vector<ZIntPoint> posArray;
  for (const ZDvidSynapse &synapse : synapseArray) {
    if (result.count(synapse.getPosition()) == 0) {
      posArray.push_back(synapse.getPosition());
    }
    const std::vector<ZIntPoint> &ptArray = synapse.getPartners();
    for (const ZIntPoint &pt : ptArray) {
      if (result.count(pt) == 0) {
        posArray.push_back(pt);
      }
    }
  }

  std::vector<uint64_t> labelArray = reader.readBodyIdAt(posArray);
  for (size_t i = 0; i < labelArray.size(); ++i) {
    result[posArray[i]] = labelArray[i];
  }

  return result;
}

bool FlyEmDataReader::IsSkeletonSynced(
    const ZDvidReader &reader, uint64_t bodyId)
{
  if (reader.hasBody(bodyId)) {
    int64_t bodyMod = reader.readBodyMutationId(bodyId);
    std::unique_ptr<ZSwcTree> tree =
        std::unique_ptr<ZSwcTree>(reader.readSwc(bodyId));
    if (!tree) {
      return false;
    } else {
      if (bodyMod < 0) { //no mutation ID exists
                         //taken as synced as long as the skeleton exists
          return true;
      } else {
        return (bodyMod == flyem::GetMutationId(tree.get()));
      }
    }

  }

  return true;
}

#if 0
std::vector<ZDvidSynapse> FlyEmDataReader::ReadSynapse(
    const ZDvidReader &reader,
    const ZIntCuboid &box, dvid::EAnnotationLoadMode mode)
{
  ZDvidUrl dvidUrl(reader.getDvidTarget());
  ZJsonArray obj = reader.readJsonArray(dvidUrl.getSynapseUrl(box));

  std::vector<ZDvidSynapse> synapseArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    synapseArray[i].loadJsonObject(synapseJson, mode);
  }

  return synapseArray;
}

std::vector<ZDvidSynapse> FlyEmDataReader::ReadSynapse(
    const ZDvidReader &reader, uint64_t label, dvid::EAnnotationLoadMode mode)
{
  ZDvidUrl dvidUrl(reader.getDvidTarget());

  ZJsonArray obj = reader.readJsonArray(
        dvidUrl.getSynapseUrl(label, mode != dvid::EAnnotationLoadMode::NO_PARTNER));

  std::vector<ZDvidSynapse> synapseArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    synapseArray[i].loadJsonObject(synapseJson, mode);
    synapseArray[i].setBodyId(label);
  }

  return synapseArray;
}

std::vector<ZDvidSynapse> FlyEmDataReader::ReadSynapse(
    const ZDvidReader &reader, uint64_t label, const ZDvidRoi &roi,
    dvid::EAnnotationLoadMode mode)
{
  ZDvidUrl dvidUrl(reader.getDvidTarget());

  ZJsonArray obj = reader.readJsonArray(
        dvidUrl.getSynapseUrl(label, mode != dvid::EAnnotationLoadMode::NO_PARTNER));

  std::vector<ZDvidSynapse> synapseArray;

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZIntPoint position = ZDvidAnnotation::GetPosition(synapseJson);
    if (roi.contains(position)) {
      synapseArray.resize(synapseArray.size() + 1);
      synapseArray.back().loadJsonObject(synapseJson, mode);
      synapseArray.back().setBodyId(label);
    }
  }

  return synapseArray;
}

ZDvidSynapse FlyEmDataReader::ReadSynapse(
    const ZDvidReader &reader, int x, int y, int z, dvid::EAnnotationLoadMode mode)
{
  std::vector<ZDvidSynapse> synapseArray =
      FlyEmDataReader::ReadSynapse(reader, ZIntCuboid(x, y, z, x, y, z), mode);
  if (!synapseArray.empty()) {
    if (synapseArray.size() > 1) {
      KWARN << QString("Duplicated synapses at (%1, %2, %3)").arg(x).arg(y).arg(z);
      synapseArray[0].setStatus(ZDvidAnnotation::EStatus::STATUS_DUPLICATED);
    }
    return synapseArray[0];
  }

  return ZDvidSynapse();
}

ZDvidSynapse FlyEmDataReader::ReadSynapse(
    const ZDvidReader &reader, const ZIntPoint &pt, dvid::EAnnotationLoadMode mode)
{
  return FlyEmDataReader::ReadSynapse(reader, pt.getX(), pt.getY(), pt.getZ(), mode);
}
#endif
