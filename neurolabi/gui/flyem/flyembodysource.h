#ifndef FLYEMBODYSOURCE_H
#define FLYEMBODYSOURCE_H

#include "common/neutudefs.h"
#include "zobject3dscan.h"

/*!
 * \brief The abstract class of getting body data
 *
 * Any data pointer returned from a method of the class should be managed by
 * the callee.
 *
 * A body is supposed to be represented by a a set of voxels (3D int points).
 * It can have lowres representation with mulitple scales, downsampled by 2 at
 * each step.
 * A body is associated with a body ID, which encodes its segmentation level and
 * segment ID.
 */
class FlyEmBodySource
{
public:
  FlyEmBodySource();
  virtual ~FlyEmBodySource();

  /*!
   * \brief Get body sparsevol data
   *
   * It returns the sparsevol of a body with encoded ID \a bodyId at the
   * downsampling scale \a dsLevel. It returns null if the requested data do not
   * exist.
   */
  virtual ZObject3dScan* getSparsevol(uint64_t bodyId, int dsLevel) const = 0;

  /*!
   * \brief Get partial body sparsevol
   *
   * \a range is the range in the original space (no downsampling). The full
   * range will be used if \a range is empty, which means it is the same as
   * no range specified.
   */
  virtual ZObject3dScan* getSparsevol(
      uint64_t bodyId, int dsLevel, const ZIntCuboid &range) const;

  /*!
   * \brief Get the coarse sparsevol data of a body
   *
   * It returns null if the requested data do not exist.
   */
  virtual ZObject3dScan* getCoarseSparsevol(uint64_t bodyId) const;

  /*!
   * \brief Get partial coarse sparsevol
   *
   * \a range is the range in the original space (no downsampling). The full
   * range will be used if \a range is empty, which means it is the same as
   * no range specified.
   */
  virtual ZObject3dScan* getCoarseSparsevol(
      uint64_t bodyId, const ZIntCuboid &range) const;

  /*!
   * \brief Get the scale of the coarse sparsevol
   *
   * If it returns 0, it means that there is no coarse sparsevol.
   */
  virtual int getCoarseSparsevolScale() const;

  /*!
   * \brief Get the block size.
   *
   * The block size is associated with the voxel scale of the coarse sparsevol
   * at level 0.
   */
  virtual ZIntPoint getBlockSize() const;

  /*!
   * \brief Get bounding box of a body.
   *
   * \return Bounding box of a body if the body exists; an empty box if it does not.
   */
  virtual ZIntCuboid getBoundBox(uint64_t bodyId) const;

  /*!
   * \brief Get bouding box of a body at a downsampling level.
   *
   * The coordinates of the box are from the downsampled space. It is not
   * necessary the same as the bounding box of the sparsevol at the same level.
   */
  virtual ZIntCuboid getBoundBox(uint64_t bodyId, int dsLevel) const;

protected:
  static neutu::EBodyLabelType GetBodyType(uint64_t bodyId);
  static uint64_t DecodeBodyId(uint64_t bodyId);
  static ZObject3dScan* PostProcess(ZObject3dScan *body, int dsLevel);
};

#endif // FLYEMBODYSOURCE_H
