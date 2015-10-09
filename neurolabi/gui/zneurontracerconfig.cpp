#include "zneurontracerconfig.h"

#include <iostream>
#include <cstring>

#include "tz_utilities.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

/////////////////////ZNeuronTracerConfig///////////////////////////

const char *ZNeuronTracerConfig::m_levelKey = "level";
const char *ZNeuronTracerConfig::m_minimalAutoScoreKey = "minimalScoreAuto";
const char *ZNeuronTracerConfig::m_minimalManualScoreKey = "minimalScoreManual";
const char *ZNeuronTracerConfig::m_minimalSeedScoreKey = "minimalScoreSeed";
const char *ZNeuronTracerConfig::m_minimal2dScoreKey = "minimalScore2d";
const char *ZNeuronTracerConfig::m_refitKey = "refit";
const char *ZNeuronTracerConfig::m_spTestKey = "spTest";
const char *ZNeuronTracerConfig::m_crossoverTestKey = "crossoverTest";
const char *ZNeuronTracerConfig::m_tuneEndKey = "tuneEnd";
const char *ZNeuronTracerConfig::m_edgePathKey = "edgePath";
const char *ZNeuronTracerConfig::m_seedMethodKey = "seedMethod";
const char *ZNeuronTracerConfig::m_recoverKey = "recover";
const char *ZNeuronTracerConfig::m_enhanceLineKey = "enhanceMask";
const char *ZNeuronTracerConfig::m_maxEucDistKey = "maxEucDist";


ZNeuronTracerConfig::ZNeuronTracerConfig()
{
  init();
}

void ZNeuronTracerConfig::init()
{
  m_minAutoScore = 0.3;
  m_minManualScore = 0.3;
  m_seedScore = 0.35;
  m_2dScore = 0.5;
  m_refit = false;
  m_spTest = true;
  m_crossoverTest = false;
  m_tuneEnd = true;
  m_edgePath = false;
  m_enhanceMask = false;
  m_seedMethod = 1;
  m_recover = 2;
  m_maxEucDist = 20;
}

void ZNeuronTracerConfig::print() const
{
  std::cout << "Minimal score (auto): " << m_minAutoScore << std::endl;
  std::cout << "Minimal score (manual): " << m_minManualScore << std::endl;
  std::cout << "Minimal score (seed): " << m_seedScore << std::endl;
  std::cout << "Minimal score (2d): " << m_2dScore << std::endl;

  std::cout << "Refit: " << m_refit << std::endl;
  std::cout << "Sp test: " << m_spTest << std::endl;
  std::cout << "Crossover test: " << m_crossoverTest << std::endl;
  std::cout << "Tuning end: " << m_tuneEnd << std::endl;
  std::cout << "Using edge path: " << m_edgePath << std::endl;
  std::cout << "Enhance line: " << m_enhanceMask << std::endl;
  std::cout << "Seeding method: " << m_seedMethod << std::endl;
  std::cout << "Recovering: " << m_recover << std::endl;
  std::cout << "Maximal gap: " << m_maxEucDist << std::endl;

  std::cout << "Levels: " << std::endl;
  for (std::map<int, ZJsonObject>::const_iterator iter = m_levelMap.begin();
       iter != m_levelMap.end(); ++iter) {
    std::cout << iter->first << std::endl;
    const ZJsonObject &obj = iter->second;
    std::cout << obj.dumpString(2) << std::endl;
  }
}

void ZNeuronTracerConfig::load(const std::string &configPath)
{
  if (fexist(configPath.c_str())) {
    ZJsonObject obj;
    obj.load(configPath);

    loadJsonObject(obj);
  }
}

ZJsonObject ZNeuronTracerConfig::getLevelJson(int level) const
{
  ZJsonObject obj;
  if (m_levelMap.empty()) {
    std::cout << "Warning: no level configuration available. "
                 "Default parameters will be used." << std::endl;
  } else {
    if (m_levelMap.count(level) > 0) {
      obj = m_levelMap.at(level);
    } else {
      std::cout << " The level " << level << " is not available." << std::endl;
      if (level < 1) {
        level = 1;
      } else if (level > 9) {
        level = 9;
      }

      obj = m_levelMap.begin()->second;
      for (std::map<int, ZJsonObject>::const_iterator iter = m_levelMap.begin();
           iter != m_levelMap.end(); ++iter) {
        if (level > iter->first) {
          std::cout << "Use level " << iter->first << " instead." << std::endl;
          obj = iter->second;
          break;
        }
      }
    }
  }

  return obj;
}

void ZNeuronTracerConfig::loadJsonObject(const ZJsonObject &jsonObj)
{
#ifdef _DEBUG_2
  std::cout << jsonObj.dumpString() << std::endl;
#endif
  if (jsonObj.hasKey("tag")) {
    if (eqstr(ZJsonParser::stringValue(jsonObj["tag"]), "trace configuration")) {
      if (jsonObj.hasKey("default")) {
        ZJsonObject defaultObj(const_cast<json_t*>(jsonObj["default"]),
            ZJsonValue::SET_INCREASE_REF_COUNT);
        if (defaultObj.hasKey(m_minimalAutoScoreKey)) {
          m_minAutoScore =
              ZJsonParser::numberValue(defaultObj[m_minimalAutoScoreKey]);
        }

        if (defaultObj.hasKey(m_minimalManualScoreKey)) {
          m_minManualScore =
              ZJsonParser::numberValue(defaultObj[m_minimalManualScoreKey]);
        }

        if (defaultObj.hasKey(m_minimalSeedScoreKey)) {
          m_seedScore =
              ZJsonParser::numberValue(defaultObj[m_minimalSeedScoreKey]);
        }

        if (defaultObj.hasKey(m_minimal2dScoreKey)) {
          m_2dScore = ZJsonParser::numberValue(defaultObj[m_minimal2dScoreKey]);
        }

        if (defaultObj.hasKey(m_maxEucDistKey)) {
          m_maxEucDist = ZJsonParser::numberValue(defaultObj[m_maxEucDistKey]);
        }

        if (defaultObj.hasKey(m_refitKey)) {
          m_refit = ZJsonParser::booleanValue(defaultObj[m_refitKey]);
        }

        if (defaultObj.hasKey(m_spTestKey)) {
          m_spTest = ZJsonParser::booleanValue(defaultObj[m_spTestKey]);
        }

        if (defaultObj.hasKey(m_crossoverTestKey)) {
          m_crossoverTest =
              ZJsonParser::booleanValue(defaultObj[m_crossoverTestKey]);
        }

        if (defaultObj.hasKey(m_tuneEndKey)) {
          m_tuneEnd =
              ZJsonParser::booleanValue(defaultObj[m_tuneEndKey]);
        }

        if (defaultObj.hasKey(m_edgePathKey)) {
          m_edgePath =
              ZJsonParser::booleanValue(defaultObj[m_edgePathKey]);
        }

        if (defaultObj.hasKey(m_seedMethodKey)) {
          m_seedMethod =
              ZJsonParser::integerValue(defaultObj[m_seedMethodKey]);
        }

        if (defaultObj.hasKey(m_recoverKey)) {
          m_recover =
              ZJsonParser::integerValue(defaultObj[m_recoverKey]);
        }

        if (defaultObj.hasKey(m_enhanceLineKey)) {
          m_enhanceMask =
              ZJsonParser::booleanValue(defaultObj[m_enhanceLineKey]);
        }
      }

      if (jsonObj.hasKey("level")) {
        const char *key;
        json_t *value;
        json_t *obj = const_cast<json_t*>(jsonObj["level"]);
        json_object_foreach(obj, key, value) {
          if (strlen(key) == 1) {
            if (key[0] >= '1' && key[0] <= '9') {
              int level = key[0] - '0';
              ZJsonObject levelObj(value, ZJsonValue::SET_INCREASE_REF_COUNT);
              m_levelMap[level] = levelObj;
#ifdef _DEBUG_
              std::cout << "Tracing leve config: " << key[0] << std::endl;
              levelObj.print();
#endif
            }
          }
        }
      }
    }
  }
}
/////////////////////////////////////////////

