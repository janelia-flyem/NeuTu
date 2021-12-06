#ifndef ZFLYEMBODYMANAGER_H
#define ZFLYEMBODYMANAGER_H

#include <QSet>
#include <QMap>

#include "flyembodyconfig.h"

/*!
 * \brief The class of managing a set of bodies for 3d display
 *
 * The class is mainly used in ZFlyEmBody3dDoc, which needs to load or unload
 * bodies for exploring neuron segmentations. A body can be registered or
 * unregistered in a manager object. A body can also be registered as a set of
 * supervoxels. Therefore, the entry of each body is stored as a mapping from
 * a normal body ID, which is always decoded, to a set of supervoxel IDs. Mapping
 * to an empty set means the body is registered as a normal body.
 */
class ZFlyEmBodyManager
{
public:
  ZFlyEmBodyManager();

  bool isEmpty() const;

  /*!
   * \brief Register a body
   *
   * Nothing will be done if a body has already been registered. It can also be
   * used to register an orphan supervoxel when \a id is encoded as a supervoxel.
   */
  void registerBody(uint64_t id);

  /*!
   * \brief Register a body with its subbody composition.
   *
   * \a comp will overwrite the old composition of \a id if it has already been
   * registered. \a id is registered as a normal body if \a comp is empty.
   */
  void registerBody(uint64_t id, const QSet<uint64_t> &comp);

  /*!
   * \brief Register a body (usually supervoxel) with its agglomeration ID.
   *
   * It adds \a bodyId to existing \a aggloID mapping or creates a new map if
   * the mapping doesn't exist.
   */
  void registerBody(uint64_t aggloId, uint64_t bodyId);

  /*!
   * \brief Deregister a body
   *
   * Remove a body from the manager. If \a id has a non-empty mapped set, the set
   * will be removed as well.
   */
  void deregisterBody(uint64_t id);

  /*!
   * \brief Register a supervoxel as an orphan (without agglomeration)
   *
   */
  void registerSupervoxel(uint64_t id);
  void deregisterSupervoxel(uint64_t id);

  bool contains(uint64_t id) const;
  bool hasMapping(uint64_t id) const;
  uint64_t getAggloId(uint64_t bodyId) const;
  QSet<uint64_t> getMappedSet(uint64_t bodyId) const;

  /*!
   * \brief Register a body with its subbody composition as being buffered
   * (for prefetching)
   *
   * \a comp will overwrite the old composition of \a id if it has already been
   * registered. \a id is registered as a normal body if \a comp is empty.
   */

  bool buffered(uint64_t id) const;
  void registerBufferedBody(uint64_t id);
  void registerBufferedBody(uint64_t id, const QSet<uint64_t> &comp);
  void deregisterBufferedBody(uint64_t id);
  QSet<uint64_t> getBufferedMappedSet(uint64_t bodyId) const;

  QSet<uint64_t> getNormalBodySet() const;
  QSet<uint64_t> getUnmappedBodySet() const;
  QSet<uint64_t> getOrphanSupervoxelSet(bool resultEncoded) const;

  /*!
   * \brief Get the ID when only one body or supervoxel presents.
   *
   * The function always returns 0 when there is a mapped body. If there is no
   * mapped body, the number of bodies is counted as the sum of the number of
   * unmapped bodies and the number of orphan supervoxels. It returns the ID of
   * the body or supervoxel if the count is 1. The returned supervoxel ID will be
   * encoded.
   */
  uint64_t getSingleBodyId() const;


  /*!
   * \brief Erase a body.
   *
   * It erases \a bodyId from the mapped sets.
   */
  void eraseSupervoxel(uint64_t bodyId);

  /*!
   * \brief Check if an ID is a supervoxel
   *
   * It returns true iff \a bodyId is not encoded as a tar or non-supervoxel
   * level and stored as a supervoxel in the body manager.
   *
   */
  bool isSupervoxel(uint64_t bodyId) const;

  bool isOrphanSupervoxel(uint64_t bodyId) const;


  QSet<uint64_t> getSupervoxelToAdd(
      const QSet<uint64_t> &bodySet, bool resultEncoded);
  QSet<uint64_t> getSupervoxelToRemove(
      const QSet<uint64_t> &bodySet, bool resultEncoded);


  void clear();

  void print() const;

  void setTodoLoaded(uint64_t bodyId);
  void setSynapseLoaded(uint64_t bodyId);
  void setSynapseLoaded(uint64_t bodyId, bool on);

  bool isTodoLoaded(uint64_t bodyId) const;
  bool isSynapseLoaded(uint64_t bodyId) const;

  /*!
   * \brief Add configuration for body update
   *
   * \a config will be added even if the body specified by \a config has not been
   * registered, unless the ID is 0. Deregistering a body will remove its config
   * as well.
   */
  void addBodyConfig(const FlyEmBodyConfig &config);

  FlyEmBodyConfig getBodyConfig(uint64_t bodyId) const;

  // The instances referred to by ZDvidUrl::getMeshesTarsUrl() represent data that
  // uses the body's identifier in multiple ways: for multiple meshes, at different
  // levels in the agglomeration history, and as a key whose associated value is a
  // tar file of meshes.  These distinct identifiers are created by encoding a
  // raw body identifier.

  /*!
   * \brief Encode a body ID.
   *
   * It returns 0 if \a rawId is an encoded ID.
   */
  static uint64_t Encode(uint64_t rawId, unsigned int level = 0, bool tar = true);
  static uint64_t EncodeTar(uint64_t rawId);

  /*!
   * \brief Encode supervoxel
   *
   * No tar encoding is applied to a supervoxel. In fact, the whole tar encoding
   * option might be removed in the future because it seems redundant. It returns
   * 0 if \a rawId is an encoded ID.
   */
  static uint64_t EncodeSupervoxel(uint64_t rawId);

  static uint64_t Decode(uint64_t encodedId);
  static bool EncodesTar(uint64_t bodyId);
  static bool EncodingSupervoxel(uint64_t bodyId);
  static unsigned int EncodedLevel(uint64_t bodyId);
  static bool EncodingSupervoxelTar(uint64_t bodyId);
  static bool IsEncoded(uint64_t bodyId);

private:
  static bool couldBeSupervoxelLevel(uint64_t bodyId);
  static bool couldBeSupervoxel(uint64_t bodyId);

private:
  QMap<uint64_t, QSet<uint64_t>> m_bodyMap;
  QMap<uint64_t, FlyEmBodyConfig> m_bodyConfigMap; //Hints for body update
  QSet<uint64_t> m_todoLoaded;
  QSet<uint64_t> m_synapseLoaded;
  QMap<uint64_t, QSet<uint64_t>> m_bufferedBodyMap;
};

#endif // ZFLYEMBODYMANAGER_H
