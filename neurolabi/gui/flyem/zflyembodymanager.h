#ifndef ZFLYEMBODYMANAGER_H
#define ZFLYEMBODYMANAGER_H

#include <QSet>
#include <QMap>

#include "zflyembodyconfig.h"

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
   * Nothing will be done if a body has already been registered.
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

  QSet<uint64_t> getNormalBodySet() const;
  QSet<uint64_t> getUnmappedBodySet() const;
  QSet<uint64_t> getOrphanSupervoxelSet() const;

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

  bool isSupervoxel(uint64_t bodyId) const;
  bool isOrphanSupervoxel(uint64_t bodyId) const;


  //Note: IDs in the the returned set are encoded.
  QSet<uint64_t> getSupervoxelToAdd(
      const QSet<uint64_t> &bodySet, bool resultEncoded);
  QSet<uint64_t> getSupervoxelToRemove(
      const QSet<uint64_t> &bodySet, bool resultEncoded);


  void clear();

  void print() const;

  void setTodoLoaded(uint64_t bodyId);
  void setSynapseLoaded(uint64_t bodyId);

  bool isTodoLoaded(uint64_t bodyId) const;
  bool isSynapseLoaded(uint64_t bodyId) const;

  /*!
   * \brief Add configuration for body update
   *
   * \a config will be added even if the body specified by \a config has not been
   * registered, unless the ID is 0. Deregistering a body will remove its config
   * as well.
   */
  void addBodyConfig(const ZFlyEmBodyConfig &config);

  ZFlyEmBodyConfig getBodyConfig(uint64_t bodyId) const;

  // The instances referred to by ZDvidUrl::getMeshesTarsUrl() represent data that
  // uses the body's identifier in multiple ways: for multiple meshes, at different
  // levels in the agglomeration history, and as a key whose associated value is a
  // tar file of meshes.  These distinct identifiers are created by encoding a
  // raw body identifier.

  static uint64_t encode(uint64_t rawId, unsigned int level = 0, bool tar = true);

  /*!
   * \brief Encode supervoxel
   *
   * No tar encoding is applied to a supervoxel. In fact, the whole tar encoding
   * option might be removed in the future because it seems redundant.
   */
  static uint64_t encodeSupervoxel(uint64_t rawId);

  static uint64_t decode(uint64_t encodedId);
  static bool encodesTar(uint64_t id);
  static bool encodingSupervoxel(uint64_t id);
  static unsigned int encodedLevel(uint64_t id);

private:
  QMap<uint64_t, QSet<uint64_t>> m_bodyMap;
  QMap<uint64_t, ZFlyEmBodyConfig> m_bodyConfigMap; //Hints for body update
  QSet<uint64_t> m_todoLoaded;
  QSet<uint64_t> m_synapseLoaded;

};

#endif // ZFLYEMBODYMANAGER_H
