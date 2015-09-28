#ifndef ZNEURONTRACERCONFIG_H
#define ZNEURONTRACERCONFIG_H

#include <string>
#include <map>

#include "zjsonobject.h"

class ZNeuronTracerConfig
{
public:
  ZNeuronTracerConfig();

  static ZNeuronTracerConfig& getInstance() {
    static ZNeuronTracerConfig config;

    return config;
  }

  void load(const std::string &configPath);
  void loadJsonObject(const ZJsonObject &jsonObj);

  double getMinAutoScore() const { return m_minAutoScore; }
  double getMinManualScore() const { return m_minManualScore; }
  double getMinSeedScore() const { return m_seedScore; }
  double getMin2dScore() const { return m_2dScore; }
  bool isRefit() const { return m_refit; }
  bool spTest() const { return m_spTest; }
  bool crossoverTest() const { return m_crossoverTest; }
  bool tuningEnd() const { return m_tuneEnd; }
  bool usingEdgePath() const { return m_edgePath; }
  bool enhancingMask() const { return m_enhanceMask; }
  int getSeedMethod() const { return m_seedMethod; }
  int getRecoverLevel() const { return m_recover; }
  double getMaxEucDist() const { return m_maxEucDist; }

  ZJsonObject getLevelJson(int level) const;

  static const char * getLevelKey() { return m_levelKey; }
  static const char * getMinimalAutoScoreKey() { return m_minimalAutoScoreKey; }
  static const char * getMinimalManualScoreKey() { return m_minimalManualScoreKey; }
  static const char * getMinimalSeedScoreKey() { return m_minimalSeedScoreKey; }
  static const char * getMinimal2dScoreKey() { return m_minimal2dScoreKey; }
  static const char * getRefitKey() { return m_refitKey; }
  static const char * getSpTestKey() { return m_spTestKey; }
  static const char * getCrossoverTestKey() { return m_crossoverTestKey; }
  static const char * getTuneEndKey() { return m_tuneEndKey; }
  static const char * getEdgePathKey() { return m_edgePathKey; }
  static const char * getSeedMethodKey() { return m_seedMethodKey; }
  static const char * getRecoverKey() { return m_recoverKey; }
  static const char * getEnhanceLineKey() { return m_enhanceLineKey; }

  void print() const;

private:
  void init();

private:
  double m_minAutoScore;
  double m_minManualScore;
  double m_seedScore;
  double m_2dScore;
  bool m_refit;
  bool m_spTest;
  bool m_crossoverTest;
  bool m_tuneEnd;
  bool m_edgePath;
  bool m_enhanceMask;
  int m_seedMethod;
  int m_recover;
  double m_maxEucDist;
  std::map<int, ZJsonObject> m_levelMap;

  static const char *m_levelKey;
  static const char *m_minimalAutoScoreKey;
  static const char *m_minimalManualScoreKey;
  static const char *m_minimalSeedScoreKey;
  static const char *m_minimal2dScoreKey;
  static const char *m_refitKey;
  static const char *m_spTestKey;
  static const char *m_crossoverTestKey;
  static const char *m_tuneEndKey;
  static const char *m_edgePathKey;
  static const char *m_seedMethodKey;
  static const char *m_recoverKey;
  static const char *m_enhanceLineKey;
  static const char *m_maxEucDistKey;
};
#endif // ZNEURONTRACERCONFIG_H
