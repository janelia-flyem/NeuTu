#ifndef ZFLYEMMB6ANALYZER_H
#define ZFLYEMMB6ANALYZER_H

#include <QMap>
#include <QVector>
#include <QSet>

#include "tz_stdint.h"

class ZDvidReader;
class ZIntPoint;
class ZDvidSynapse;

class ZFlyEmMB6Analyzer
{
public:
  ZFlyEmMB6Analyzer();

  void setDvidReader(ZDvidReader *reader);

  template <typename InputIterator>
  void updateBodyName(const InputIterator &first, const InputIterator last);

  void updateBodyName(uint64_t bodyId);

  void setBodyId(uint64_t bodyId);

  QString getBodyName(uint64_t bodyId);
  QString getPunctumName(uint64_t preBodyId, uint64_t postBodyId);
  QString getPunctumName(const QString &preName, const QString &postName) const;
  QString getPunctumName(const ZIntPoint &pt);
  QString getPunctumName(const ZDvidSynapse &synapse);

  bool isKcNeuron(const QString &name) const;

private:
  void init();

private:
  ZDvidReader *m_reader;
  QMap<uint64_t, QString> m_bodyNameMap;
  QSet<QString> m_kcName;
};

template <typename InputIterator>
void ZFlyEmMB6Analyzer::updateBodyName(
    const InputIterator &first, const InputIterator last)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    updateBodyName(*iter);
  }
}

#endif // ZFLYEMMB6ANALYZER_H
