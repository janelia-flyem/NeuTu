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

  /*!
   * \brief Set the resolution range
   *
   * It gives hints about the resolution range that the factory can make. A
   * derived class can decide how to use the range.
   */
  void setResRange(int minResLevel, int maxResLevel);
  void setMinResLevel(int level);
  void setMaxResLevel(int level);

protected:
  virtual FlyEmBodyMesh make_(const FlyEmBodyConfig &config) = 0;

  int m_minResLevel = 0;
  int m_maxResLevel = 30;

private:
  std::function<void(FlyEmBodyMesh&)> _postProcess;
};

#endif // FLYEMBODYMESHFACTORY_H
