/*
 * Copyright (C) 2005-2012 University of Muenster, Germany.
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>
 * For a list of authors please refer to the file "CREDITS.txt".
 * Copyright (C) 2012-2013 Korea Institiute of Science and Technologhy, Seoul.
 * Linqing Feng, Jinny Kim's lab <http://jinny.kist.re.kr>
 *
 * This file is derived from code of the free Voreen software package.
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License in the file
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef Z3DPICKINGMANAGER_H
#define Z3DPICKINGMANAGER_H

#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>
#include <QList>

#include "z3drendertarget.h"
#include "zglmutils.h"
//#include "swctreenode.h"

class Z3DPickingManager
{
public:
  // input render target should have color internal format as GL_RGBA8
  // must call
  void setRenderTarget(Z3DRenderTarget& rt);

  // must call
  void setDevicePixelRatio(double dpr)
  { m_devicePixelRatio = dpr; }

  glm::col4 registerObject(const void* obj);

  void deregisterObject(const void* obj);

//  void deregisterObject(const std::vector<Swc_Tree_Node*> &nodeList);

  void deregisterObject(const glm::col4& col);

  void clearRegisteredObjects();

  glm::col4 colorOfObject(const void* obj);

  glm::vec4 fColorOfObject(const void* obj)
  { return glm::vec4(glm::vec4(colorOfObject(obj)) / 255.f); }

  const void* objectOfColor(const glm::col4& col);

  const void* objectAtWidgetPos(glm::ivec2 pos);
  std::vector<const void*> objectAtWidgetPos(std::vector<glm::ivec2> &posArray);
  std::vector<const void*> objectAtWidgetPos(
      const std::vector<std::pair<int, int> > &posArray);
  std::set<const void*> objectsInWidgetRect(glm::ivec2 p0, glm::ivec2 p1);

  // find all objects within a radius of pos, sort by distance
  // if radius is -1, search the whole image
  std::vector<const void*> sortObjectsByDistanceToPos(
      const glm::ivec2& pos, int radius = -1, bool ascend = true);

  bool isHit(const glm::ivec2& pos, const void* obj)
  { return (objectAtWidgetPos(pos) == obj); }

  void bindTarget()
  { m_renderTarget->bind(); }

  void releaseTarget()
  { m_renderTarget->release(); }

  void clearTarget();

  Z3DRenderTarget& renderTarget() const
  { return *m_renderTarget; }

  bool isRegistered(const void* obj)
  { return m_objectToColor.find(obj) != m_objectToColor.end(); }

  bool isRegistered(const glm::col4& col)
  { return m_colorToObject.find(col) != m_colorToObject.end(); }

private:
  void increaseColor();

private:
  std::unordered_map<glm::col4, const void*> m_colorToObject;
  std::unordered_map<const void*, glm::col4> m_objectToColor;
  Z3DRenderTarget* m_renderTarget = nullptr;
  glm::col4 m_currentColor{0, 0, 0, 128};
  double m_devicePixelRatio = 0;
};

#endif // Z3DPICKINGMANAGER_H
