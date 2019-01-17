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

#include "z3dpickingmanager.h"

#include <QApplication>
#include <functional>
#include <memory>
#include <vector>
#include <QWidget>

#include "z3dgl.h"
#include "z3dtexture.h"
#include "logging/zqslog.h"

void Z3DPickingManager::setRenderTarget(Z3DRenderTarget& rt)
{
  CHECK(rt.attachment(GL_COLOR_ATTACHMENT0)->internalFormat() == GL_RGBA8);
  m_renderTarget = &rt;
}

glm::col4 Z3DPickingManager::registerObject(const void* obj)
{
  increaseColor();
  m_colorToObject[m_currentColor] = obj;
  m_objectToColor[obj] = m_currentColor;
  return m_currentColor;
}

void Z3DPickingManager::deregisterObject(const void* obj)
{
  glm::col4 col = colorOfObject(obj);
  m_colorToObject.erase(col);
  m_objectToColor.erase(obj);
}

/*
void Z3DPickingManager::deregisterObject(
    const std::vector<Swc_Tree_Node *> &nodeList)
{
  std::vector<glm::col4> colList(nodeList.size());
  for (size_t i = 0; i < nodeList.size(); ++i) {
    colList[i] = colorOfObject(nodeList[i]);
  }

  m_colorToObject.erase(colList.begin(), colList.end());
  m_objectToColor.erase(nodeList.begin(), nodeList.end());
}
*/

void Z3DPickingManager::deregisterObject(const glm::col4& col)
{
  const void* obj = objectOfColor(col);
  m_colorToObject.erase(col);
  m_objectToColor.erase(obj);
}

void Z3DPickingManager::clearRegisteredObjects()
{
  m_colorToObject.clear();
  m_objectToColor.clear();
  m_currentColor = glm::col4(0, 0, 0, 128);
}

glm::col4 Z3DPickingManager::colorOfObject(const void* obj)
{
  if (!obj)
    return glm::col4(0, 0, 0, 0);

  if (isRegistered(obj)) {
    return m_objectToColor[obj];
  } else
    return glm::col4(0, 0, 0, 0);
}

const void* Z3DPickingManager::objectOfColor(const glm::col4& col)
{
  if (col.a == 0)
    return nullptr;

  if (isRegistered(col)) {
    return m_colorToObject[col];
  } else {
    return nullptr;
  }
}

std::vector<const void*> Z3DPickingManager::objectAtWidgetPos(
    std::vector<glm::ivec2> &posArray)
{
  std::vector<const void*> objArray;

  assert(m_devicePixelRatio >= 1);
  glm::ivec3 texSize = glm::ivec3(
        m_renderTarget->attachment(GL_COLOR_ATTACHMENT0)->dimension());

  for (glm::ivec2 &pos : posArray) {
    pos[0] = pos[0] * m_devicePixelRatio;
    pos[1] = pos[1] * m_devicePixelRatio;
    pos[1] = texSize[1] - pos[1];
  }

  std::vector<glm::col4> colorArray = m_renderTarget->colorAtPos(posArray);

  for (const glm::col4 &color : colorArray) {
    objArray.push_back(objectOfColor(color));
  }

  return objArray;
}

std::vector<const void*> Z3DPickingManager::objectAtWidgetPos(
    const std::vector<std::pair<int, int> > &posArray)
{
  std::vector<glm::ivec2> vecArray;
  for (const std::pair<int, int> &pos : posArray) {
    vecArray.emplace_back(pos.first, pos.second);
  }

  return objectAtWidgetPos(vecArray);
}

const void* Z3DPickingManager::objectAtWidgetPos(glm::ivec2 pos)
{
  assert(m_devicePixelRatio >= 1);
  pos[0] = pos[0] * m_devicePixelRatio;
  pos[1] = pos[1] * m_devicePixelRatio;

  glm::ivec3 texSize = glm::ivec3(m_renderTarget->attachment(GL_COLOR_ATTACHMENT0)->dimension());
  pos[1] = texSize[1] - pos[1];
  return objectOfColor(m_renderTarget->colorAtPos(pos));
}

std::set<const void*> Z3DPickingManager::objectsInWidgetRect(glm::ivec2 p0, glm::ivec2 p1)
{
  assert(m_devicePixelRatio >= 1);
  p0[0] = p0[0] * m_devicePixelRatio;
  p0[1] = p0[1] * m_devicePixelRatio;
  p1[0] = p1[0] * m_devicePixelRatio;
  p1[1] = p1[1] * m_devicePixelRatio;

  glm::ivec3 texSize = glm::ivec3(m_renderTarget->attachment(GL_COLOR_ATTACHMENT0)->dimension());
  p0[1] = texSize[1] - p0[1];
  p1[1] = texSize[1] - p1[1];
  std::vector<glm::col4> colors = m_renderTarget->colorsInRect(p0, p1);

  std::set<const void*> objects;
  for (const glm::col4 &color : colors) {
    objects.insert(objectOfColor(color));
  }
  return objects;
}

std::vector<const void*> Z3DPickingManager::sortObjectsByDistanceToPos(const glm::ivec2& pos, int radius, bool ascend)
{
  std::map<glm::col4, int, Col4Compare> col2dist;
  const Z3DTexture* tex = m_renderTarget->attachment(GL_COLOR_ATTACHMENT0);
  GLenum dataFormat = GL_BGRA;
  GLenum dataType = GL_UNSIGNED_INT_8_8_8_8_REV;
  auto buf = std::make_unique<glm::col4[]>(tex->bypePerPixel(dataFormat, dataType) * tex->numPixels() / 4);
  tex->downloadTextureToBuffer(dataFormat, dataType, buf.get());
  glm::ivec2 texSize = glm::ivec2(m_renderTarget->size());
  if (radius < 0)
    radius = std::max(texSize.x, texSize.y);
  for (int y = std::max(0, pos.y - radius); y <= std::min(texSize.y - 1, pos.y + radius); ++y) {
    for (int x = std::max(0, pos.x - radius); x <= std::min(texSize.x - 1, pos.x + radius); ++x) {
      glm::col4 col = buf[(y * texSize.x) + x];
      std::swap(col.r, col.b);
      if (col2dist[col] == 0)
        col2dist[col] = (x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y);
      else
        col2dist[col] = std::min(col2dist[col], (x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y));
    }
  }
  std::vector<const void*> res;
  if (ascend) {
    std::multimap<int, const void*> dist2obj;
    for (std::map<glm::col4, int>::const_iterator it = col2dist.begin();
         it != col2dist.end(); ++it) {
      const void* obj = objectOfColor(it->first);
      if (obj)
        dist2obj.emplace(it->second, obj);
    }
    for (std::multimap<int, const void*>::const_iterator it = dist2obj.begin();
         it != dist2obj.end(); ++it) {
      res.push_back(it->second);
    }
  } else {
    std::multimap<int, const void*, std::greater<int>> dist2obj;
    for (std::map<glm::col4, int>::const_iterator it = col2dist.begin();
         it != col2dist.end(); ++it) {
      const void* obj = objectOfColor(it->first);
      if (obj)
        dist2obj.emplace(it->second, obj);
    }
    for (std::multimap<int, const void*>::const_iterator it = dist2obj.begin();
         it != dist2obj.end(); ++it) {
      res.push_back(it->second);
    }
  }
  return res;
}

void Z3DPickingManager::clearTarget()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Z3DPickingManager::increaseColor()
{
  uint32_t col = bit_cast<uint32_t>(m_currentColor);
  if (col != 0xffffffff) {
    ++col;
    m_currentColor = bit_cast<glm::col4>(col);
  } else {
    m_currentColor = glm::col4(0, 0, 0, 128);
    //LOG(ERROR) << "Out of colors...";
  }
}
