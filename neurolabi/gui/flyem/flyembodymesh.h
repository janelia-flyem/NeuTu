#ifndef FLYEMBODYMESH_H
#define FLYEMBODYMESH_H

#include <memory>

#include "zuncopyable.h"
#include "flyembodyconfig.h"
#include "zmesh.h"

/*!
 * \brief The class for hosting body meshes
 *
 * The mesh object contained in an instance of the class is owned by the instance
 * until it is released.
 */
class FlyEmBodyMesh
{
public:
  FlyEmBodyMesh(ZMesh *mesh, const FlyEmBodyConfig &config);
  FlyEmBodyMesh(std::unique_ptr<ZMesh> &&mesh, const FlyEmBodyConfig &config);
  FlyEmBodyMesh(const FlyEmBodyMesh &mesh) = delete;
  FlyEmBodyMesh(FlyEmBodyMesh &&mesh) = default;

  bool hasData() const;
  ZMesh* getData() const;

  ZMesh* releaseData();
  FlyEmBodyConfig getBodyConfig() const;

  void setDsLevel(int level);
  void disableNextDsLevel();

private:
  std::unique_ptr<ZMesh> m_mesh;
  FlyEmBodyConfig m_config;
};

#endif // FLYEMBODYMESH_H
