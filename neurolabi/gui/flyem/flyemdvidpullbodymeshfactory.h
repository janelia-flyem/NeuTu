#ifndef FLYEMDVIDPULLBODYMESHFACTORY_H
#define FLYEMDVIDPULLBODYMESHFACTORY_H

#include <functional>

#include "flyembodymeshfactory.h"

class ZDvidReader;

/*!
 * \brief The class of reading body meshes from dvid
 */
class FlyEmDvidPullBodyMeshFactory : public FlyEmBodyMeshFactory
{
public:
  FlyEmDvidPullBodyMeshFactory();
  ~FlyEmDvidPullBodyMeshFactory();

  /*!
   * \brief Set the DVID reader
   *
   * Set the DVID reader to \a reader. \a dispose is a disposer that applies on
   * \a reader when the object is destructed.
   *
   * Note: the function takes a raw pointer and a diposer instead of a smart
   *       pointer for better compatibility.
   */
  void setReader(
      ZDvidReader *reader, std::mutex *readerMutex,
      std::function<void(ZDvidReader*)> dispose);

  void useNgMesh(bool on);
  void useObjMesh(bool on);

protected:
  FlyEmBodyMesh make_(const FlyEmBodyConfig &config) override;

private:
  int getCoarseBodyZoom() const;

private:
  ZDvidReader *m_reader = nullptr;
  mutable std::mutex *m_readerMutex = nullptr;
  std::function<void(ZDvidReader*)> m_disposeReader = nullptr;
  bool m_usingNgMesh = false;
  bool m_usingObjMesh = false;
};

#endif // FLYEMDVIDPULLBODYMESHFACTORY_H
