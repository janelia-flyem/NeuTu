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
#include "QsLog.h"

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

const void* Z3DPickingManager::getObjectAtWidgetPos(
    glm::ivec2 pos, glm::ivec3 texSize, int dpr)
{
#ifdef _QT5_
#if 0
  std::cout << "device pixel ratio: "
            << QApplication::activeWindow()->devicePixelRatio() << std::endl;
#endif
  pos[0] = pos[0] * dpr;
  pos[1] = pos[1] * dpr;

#ifdef _DEBUG_
  std::cout << "Calibrated by " << dpr << ": "
            << pos[0] << ", " << pos[1] << std::endl;
#endif
#endif
//  glm::ivec3 texSize =
//      getRenderTarget()->getAttachment(GL_COLOR_ATTACHMENT0)->getDimensions();

#ifdef _DEBUG_
  std::cout << "Tex size: " << texSize[0] << ", " << texSize[1] << ", "
            << texSize[2] << std::endl;
#endif

  pos[1] = texSize[1]- pos[1];
  return getObjectAtPos(pos);
}

const void* Z3DPickingManager::getObjectAtPos(glm::ivec2 pos)
{
  return objectOfColor(m_renderTarget->colorAtPos(pos));
}


const void* Z3DPickingManager::getObjectAtWidgetPos(glm::ivec2 pos, glm::ivec3 texSize)
{
  int dpr = QApplication::activeWindow()->devicePixelRatio();
  return getObjectAtWidgetPos(pos, texSize, dpr);
}

const void* Z3DPickingManager::objectAtWidgetPos(glm::ivec2 pos)
{
  pos[0] = pos[0] * qApp->devicePixelRatio();
  pos[1] = pos[1] * qApp->devicePixelRatio();

  glm::ivec3 texSize = glm::ivec3(m_renderTarget->attachment(GL_COLOR_ATTACHMENT0)->dimension());
  pos[1] = texSize[1] - pos[1];
  return objectOfColor(m_renderTarget->colorAtPos(pos));
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
