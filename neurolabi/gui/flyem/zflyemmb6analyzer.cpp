#include "zflyemmb6analyzer.h"

#include <QString>

#include "dvid/zdvidreader.h"
#include "zflyembodyannotation.h"

ZFlyEmMB6Analyzer::ZFlyEmMB6Analyzer()
{
  init();
}

void ZFlyEmMB6Analyzer::init()
{
  m_reader = NULL;

  m_kcName.insert("KC-s");
  m_kcName.insert("KC-p");
  m_kcName.insert("KC-c");
  m_kcName.insert("KC-any");
}

void ZFlyEmMB6Analyzer::setDvidReader(ZDvidReader *reader)
{
  m_reader = reader;
}

void ZFlyEmMB6Analyzer::updateBodyName(uint64_t bodyId)
{
  if (!m_bodyNameMap.contains(bodyId)) {
    ZFlyEmBodyAnnotation annotation = m_reader->readBodyAnnotation(bodyId);
    m_bodyNameMap[bodyId] = annotation.getName().c_str();
  }
}

QString ZFlyEmMB6Analyzer::getBodyName(uint64_t bodyId)
{
  updateBodyName(bodyId);

  return m_bodyNameMap[bodyId];
}

bool ZFlyEmMB6Analyzer::isKcNeuron(const QString &name) const
{
  return m_kcName.contains(name);
//  return name.startsWith("KC");
}

QString ZFlyEmMB6Analyzer::getPunctumName(
    uint64_t preBodyId, uint64_t postBodyId)
{
  QString preName = getBodyName(preBodyId);
  QString postName = getBodyName(postBodyId);

  return getPunctumName(preName, postName);
}

QString ZFlyEmMB6Analyzer::getPunctumName(
    const QString &preName, const QString &postName) const
{

  if (preName.isEmpty()) {
    return postName;
  }

  if (postName.isEmpty()) {
    return preName;
  }

  if (isKcNeuron(preName) && isKcNeuron(postName)) {
    return "KC_KC";
  }

  return preName + "_" + postName;
}

QString ZFlyEmMB6Analyzer::getPunctumName(const ZIntPoint &/*pt*/)
{
  return "";
}

QString ZFlyEmMB6Analyzer::getPunctumName(const ZDvidSynapse &synapse)
{
  QString name;

  std::vector<ZIntPoint> partners;
  ZJsonArray jsonArray = synapse.getRelationJson();
  for (size_t i = 0; i < jsonArray.size(); ++i) {
    ZJsonObject jsonObj(jsonArray.value(i));
    if (jsonObj.hasKey("To")) {
      ZJsonArray ptJson(jsonObj.value("To"));
      ZIntPoint pt;
      pt.set(ZJsonParser::integerValue(ptJson.at(0)),
             ZJsonParser::integerValue(ptJson.at(1)),
             ZJsonParser::integerValue(ptJson.at(2)));
      partners.push_back(pt);
    }
  }

  QString bodyName = getBodyName(synapse.getBodyId());

  if (!partners.empty()) {
    std::vector<uint64_t> bodyIdArray =
        m_reader->readBodyIdAt(partners.begin(), partners.end());

    if (synapse.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
      QString partnerName;
      for (std::vector<uint64_t>::const_iterator iter = bodyIdArray.begin();
           iter != bodyIdArray.end(); ++iter) {
        uint64_t partnerId = *iter;
        partnerName = getBodyName(partnerId);
        if (isKcNeuron(partnerName)) {
          break;
        }
      }

      name = getPunctumName(bodyName, partnerName);
    } else {
      name = getPunctumName(getBodyName(bodyIdArray.front()), bodyName);
    }
  }

  return name;
}
