#ifndef ZSTACKDOCPROXY_H
#define ZSTACKDOCPROXY_H

#include <QList>

class ZStackDoc;
class ZMesh;

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

  static void SaveMesh(ZStackDoc *doc, const QString &fileName,
                       std::function<bool(const ZMesh*)> pred);
  static void SaveVisibleMesh(ZStackDoc *doc, const QString &fileName);
};


#endif // ZSTACKDOCPROXY_H
