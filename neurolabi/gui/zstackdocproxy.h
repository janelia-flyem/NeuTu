#ifndef ZSTACKDOCPROXY_H
#define ZSTACKDOCPROXY_H

#include "zstackdoc.h"

class ZStackDocProxy
{
public:
  ZStackDocProxy();

  /*
  static QList<ZStackObject*> GetObjectList(
      ZStackDoc *doc, ZStackDoc::EDocumentDataType dataType);
      */

  static QList<ZMesh*> GetGeneralMeshList(const ZStackDoc *doc);
  static QList<ZMesh*> GetRoiMeshList(ZStackDoc *doc);
};


#endif // ZSTACKDOCPROXY_H
