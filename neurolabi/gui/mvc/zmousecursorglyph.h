#ifndef ZMOUSECURSORGLYPH_H
#define ZMOUSECURSORGLYPH_H

#include <functional>
#include <QMap>

#include "common/neutudefs.h"
#include "zstackobjectrole.h"

class ZStackObject;
class ZPoint;
class ZWeightedPoint;
class ZStroke2d;

class ZMouseCursorGlyph
{
public:
  ZMouseCursorGlyph();
  ~ZMouseCursorGlyph();

  enum ERole {
    ROLE_STROKE, ROLE_SWC, ROLE_SYNAPSE, ROLE_BOOKMARK, ROLE_TODO_ITEM
  };

  std::string GetRoleName(ERole role);

  QList<ZStackObject*> getGlyphList() const;

  bool isActivated() const;
  bool isActivated(ERole role) const;

  void setPrepareFunc(
      ERole role, std::function<void(ZStackObject*)> prepare);

  void activate(ERole role);
  void activate(
      ERole role, std::function<void(ZStackObject*)> prepare);
  void activate(
      ERole role, std::function<void(ZStackObject*, ERole)> prepare);

  void deactivate();
  void processActiveChange(std::function<void(ZStackObject*)> proc);
  void processActiveChange(
      std::function<void(ZStackObject*, ZStackObject*)> proc);

  void setActiveGlyphPosition(
      const ZPoint &pos, std::function<void(ZStackObject*)> postProc
      = std::function<void(ZStackObject*)>());
  void appendActiveGlyphPosition(
      const ZPoint &pos, std::function<void(ZStackObject*)> postProc);
  void addActiveGlyphSize(
      double dr, std::function<void(ZStackObject*)> postProc
      = std::function<void(ZStackObject*)>());
  void setActiveGlyphSize(
      double r, std::function<void(ZStackObject*)> postProc
      = std::function<void(ZStackObject*)>());

  double getActiveGlyphSize() const;
  ZPoint getActiveGlyphPosition() const;
  ZWeightedPoint getActiveGlyphGeometry() const;
  ZStackObjectRole getActiveObjectRole() const;

  void useActiveGlyph(
      std::function<void(const ZPoint &center, double radius)> proc);
  void updateActiveGlyph(std::function<void(ZStackObject*)> f);

  ZStroke2d* makeStrokeFromActiveGlyph() const;

  void setSliceAxis(neutu::EAxis axis);

public: //for testing
  ZStackObject* _getGlyph(ERole role) const;
  ZStackObject* _getActiveGlyph() const;
  template<typename T>
  T* _getGlyph(ERole role) const;

private:
  void init();
  void addGlyph(ERole role, ZStackObject *obj);

  ZStackObject* getGlyph(ERole role) const;
  ZStackObject* getActiveGlyph() const;
  template<typename T>
  T* getGlyph(ERole role) const;

private:
  QMap<ERole, ZStackObject*> m_glyphMap;
  QMap<ERole, std::function<void(ZStackObject*)>> m_prepare;
  ZStackObject *m_activeGlyph = nullptr;
  ZStackObject *m_prevActiveGlyph = nullptr;
  QMap<ERole, double> m_defaultGlyphSize;
};

template <typename T>
T* ZMouseCursorGlyph::getGlyph(ERole role) const
{
  if (m_glyphMap.contains(role)) {
    return dynamic_cast<T*>(m_glyphMap.value(role));
  }

  return nullptr;
}

template <typename T>
T* ZMouseCursorGlyph::_getGlyph(ERole role) const
{
  return getGlyph<T>(role);
}

#endif // ZMOUSECURSORGLYPH_H
