#ifndef ZFLYEMBODYMANAGER_H
#define ZFLYEMBODYMANAGER_H

#include <QSet>
#include <QMap>

class ZFlyEmBodyManager
{
public:
  ZFlyEmBodyManager();

  /*!
   * \brief Register a body
   *
   * Nothing will be done if a body has already been registered.
   */
  void registerBody(uint64_t id);

  void deregisterBody(uint64_t id);


  void registerBody(uint64_t id, const QSet<uint64_t> &comp);

  bool contains(uint64_t id) const;
  bool hasMapping(uint64_t id) const;


  static uint64_t encode(uint64_t rawId, unsigned int level = 0, bool tar = true);
  static uint64_t decode(uint64_t encodedId);
  static bool encodesTar(uint64_t id);
  static unsigned int encodedLevel(uint64_t id);

private:
  QMap<uint64_t, QSet<uint64_t>> m_bodyMap;

};

#endif // ZFLYEMBODYMANAGER_H
