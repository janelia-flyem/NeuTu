#ifndef ZSTACKWATERSHED_H
#define ZSTACKWATERSHED_H

#include <vector>

#include "tz_stack_watershed.h"
#include "tz_cuboid_i.h"

class ZStack;
class ZIntPoint;

/*!
 * \brief The class of running watershed
 */
class ZStackWatershed
{
public:
  ZStackWatershed();
  ~ZStackWatershed();

public:
  //ZStack* run(const Stack *stack, const std::vector<ZStack *> &seedMask);
  /*!
   * \brief Run seeded watershed
   *
   * \return Label feild of the watershed.
   */
  ZStack* run(const ZStack *stack, const std::vector<ZStack *> &seedMask);

  ZStack* run(const ZStack *stack, const ZStack* seedMask);

  void setRange(int x0, int y0, int z0, int x1, int y1, int z1);
  void setRange(const Cuboid_I &box);
  inline void setFloodingZero(bool status) {
    m_floodingZero = status;
  }

private:
  Stack_Watershed_Workspace* createWorkspace(const Stack *stack);
  void addSeed(Stack_Watershed_Workspace *ws, const ZIntPoint &offset,
               const std::vector<ZStack*> &seedMask);

private:
  Cuboid_I m_range;
  bool m_floodingZero;
};

#endif // ZSTACKWATERSHED_H
