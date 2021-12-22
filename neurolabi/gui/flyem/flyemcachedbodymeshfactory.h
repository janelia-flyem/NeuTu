#ifndef FLYEMCACHEDBODYMESHFACTORY_H
#define FLYEMCACHEDBODYMESHFACTORY_H

#include <memory>

#include "flyembodymeshfactory.h"

class FlyEmBodyMeshCache;

/*!
 * \brief The class for managing mesh caching workflow
 *
 * General workflow:
 *
 * make mesh from fast factory
 *  -yes-> END
 *   +no-> retrieve mesh from cache
 *     -yes-> END
 *      +no-> make mesh from slow factory
 *        -yes-> store mesh in cache -> END
 *         +no-> END
 */
class FlyEmCachedBodyMeshFactory : public FlyEmBodyMeshFactory
{
public:
  FlyEmCachedBodyMeshFactory();

  void setFastFactory(std::shared_ptr<FlyEmBodyMeshFactory> f);
  void setSlowFactory(std::shared_ptr<FlyEmBodyMeshFactory> f);
  void setCache(std::shared_ptr<FlyEmBodyMeshCache> cache);

protected:
  FlyEmBodyMesh _make(const FlyEmBodyConfig &config) override;

private:
  std::shared_ptr<FlyEmBodyMeshFactory> m_fastMeshFactory;
  std::shared_ptr<FlyEmBodyMeshFactory> m_slowMeshFactory;
  std::shared_ptr<FlyEmBodyMeshCache> m_meshCache;
};

#endif // FLYEMCACHEDBODYMESHFACTORY_H
