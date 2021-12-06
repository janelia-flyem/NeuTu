#ifndef FLYEMBODYMESHFACTORY_H
#define FLYEMBODYMESHFACTORY_H

#include <functional>

#include "flyembodyconfig.h"
#include "flyembodymesh.h"

/*!
 * \brief The abstract factory class for make body meshes
 */
class FlyEmBodyMeshFactory
{
public:
  FlyEmBodyMeshFactory();
  virtual ~FlyEmBodyMeshFactory();

  FlyEmBodyMesh make(const FlyEmBodyConfig &config);

  /*!
   * \brief Set post processing step
   *
   * \a f will be applied on the generated mesh before the mesh is returned from
   * the \a make method.
   */
  void setPostProcess(std::function<void(FlyEmBodyMesh&)> f);

protected:
  virtual FlyEmBodyMesh _make(const FlyEmBodyConfig &config) = 0;

private:
  std::function<void(FlyEmBodyMesh&)> _postProcess;
};

#endif // FLYEMBODYMESHFACTORY_H
