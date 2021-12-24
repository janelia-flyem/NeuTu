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

  /*!
   * \brief Set fast factory
   *
   * It removes the fast factory if \a f is emtpy.
   */
  void setFastFactory(std::shared_ptr<FlyEmBodyMeshFactory> f);

  /*!
   * \brief Set slow factory
   *
   * It removes the slow factory if \a f is empty.
   */
  void setSlowFactory(std::shared_ptr<FlyEmBodyMeshFactory> f);

  /*!
   * \brief Set cache
   *
   * It removes cache if \a cache is empty.
   */
  void setCache(std::shared_ptr<FlyEmBodyMeshCache> cache);

protected:
  FlyEmBodyMesh make_(const FlyEmBodyConfig &config) override;

private:
  std::shared_ptr<FlyEmBodyMeshFactory> m_fastMeshFactory;
  std::shared_ptr<FlyEmBodyMeshFactory> m_slowMeshFactory;
  std::shared_ptr<FlyEmBodyMeshCache> m_meshCache;
};

#endif // FLYEMCACHEDBODYMESHFACTORY_H
