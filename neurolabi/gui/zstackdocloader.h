#ifndef ZSTACKDOCLOADER_H
#define ZSTACKDOCLOADER_H

#include <QMap>
#include "zstackobject.h"
#include "zstackobjectrole.h"

class ZStackDoc;
class ZStackDocReader;


class ZStackDocLoader
{
public:
  ZStackDocLoader();

  enum ELoadMode {
    LOAD_OVERWRITE, LOAD_APPEND
  };

  void clear();

  void load(ZStackDoc *doc, const ZStackDocReader &reader);

  ELoadMode getLoadMode(ZStackObject::EType type) const;

  void setLoadMode(ZStackObject::EType type, ELoadMode mode);

private:
  void processObjectRemoved(ZStackDoc *doc, ZStackObject::EType type);
  void processObjectAdded(ZStackDoc *doc, ZStackObject::EType type);
  void processObjectChanged(ZStackDoc *doc, ZStackObject::EType type);
  void processPlayerChanged(ZStackDoc *doc);
  void processChanged(ZStackDoc *doc);

private:
  QMap<ZStackObject::EType, ELoadMode> m_loadType;
  QList<ZStackObject::EType> m_typeRemoved;
  QList<ZStackObject::EType> m_typeAdded;
  ZStackObjectRole m_roleRemoved;
  ZStackObjectRole m_roleAdded;
  bool m_isStackReserved;
  bool m_isSparseStackReserved;
  bool m_isStackChanged;
  bool m_isSparseStackChanged;
};

#endif // ZSTACKDOCLOADER_H
