#include "flyemdatareader.h"

#include <QByteArray>

//#include "geometry/zintpoint.h"
//#include "geometry/zintcuboid.h"

#include "zstring.h"
#include "zjsondef.h"
#include "neutubeconfig.h"
#include "zmesh.h"

#include "zjsonparser.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zobject3dscan.h"

#include "zmeshfactory.h"

#include "logging/zlog.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidbufferreader.h"
#include "zdvidutil.h"
//#include "dvid/zdvidsynapse.h"
//#include "dvid/zdvidroi.h"

#include "flyemdataconfig.h"
#include "zflyemneuronbodyinfo.h"
#include "zflyembodyannotation.h"

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
    const ZDvidReader &reader, const std::string &roiName)
{
  ZMesh *mesh = NULL;

  ZJsonObject roiInfo = reader.readJsonObjectFromKey(
        ZDvidData::GetName(ZDvidData::ERole::ROI_KEY).c_str(), roiName.c_str());
  if (roiInfo.hasKey(neutu::json::REF_KEY)) {
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
