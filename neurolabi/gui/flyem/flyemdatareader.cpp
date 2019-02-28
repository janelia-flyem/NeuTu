#include "flyemdatareader.h"

#include <QByteArray>

//#include "geometry/zintpoint.h"
//#include "geometry/zintcuboid.h"

#include "zstring.h"

#include "zjsonparser.h"
#include "zjsonarray.h"
#include "zjsonobject.h"

#include "logging/zlog.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"
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
  FlyEmDataConfig config;
  config.loadContrastProtocol(reader.readContrastProtocal());
  config.loadBodyStatusProtocol(reader.readBodyStatusV2());

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
