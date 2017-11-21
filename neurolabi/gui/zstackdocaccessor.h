#ifndef ZSTACKDOCACCESSOR_H
#define ZSTACKDOCACCESSOR_H

#include "zstackobject.h"
#include "zstackdocdatabuffer.h"

class ZStackDoc;

/*!
 * \brief The class for accessing data in ZStackDoc in a thread-safe way.
 */
class ZStackDocAccessor
{
public:
  ZStackDocAccessor();


public:
  static void RemoveObject(
      ZStackDoc *doc, ZStackObject *obj, bool deleteObject);
  static void RemoveObject(
      ZStackDoc *doc, ZStackObject::EType type, bool deleteObject);
  static void RemoveAllSwcTree(ZStackDoc *doc, bool deleteObject);
  static void AddObject(ZStackDoc *doc, ZStackObject *obj);

private:
  static ZStackDocObjectUpdate::EAction GetRemoveAction(bool deleteObject);
};

#endif // ZSTACKDOCACCESSOR_H
