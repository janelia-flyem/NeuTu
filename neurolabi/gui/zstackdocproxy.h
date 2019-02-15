#ifndef ZSTACKDOCPROXY_H
#define ZSTACKDOCPROXY_H

#include "mvc/zstackdoc.h"

class ZStackDocProxy
{
public:
  ZStackDocProxy();

  /*
  static QList<ZStackObject*> GetObjectList(
      ZStackDoc *doc, ZStackDoc::EDocumentDataType dataType);
      */

  static QList<ZMesh*> GetGeneralMeshList(const ZStackDoc *doc);
  static QList<ZMesh*> GetNonRoiMeshList(const ZStackDoc *doc);
  static QList<ZMesh*> GetBodyMeshList(const ZStackDoc *doc);
  static QList<ZMesh*> GetRoiMeshList(ZStackDoc *doc);
  static ZMesh* GetMeshForSplit(ZStackDoc *doc);
};


#endif // ZSTACKDOCPROXY_H
