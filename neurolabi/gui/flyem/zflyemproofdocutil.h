#ifndef ZFLYEMPROOFDOCUTIL_H
#define ZFLYEMPROOFDOCUTIL_H

#include <set>
#include <QString>
#include <QList>

#include "common/neutudefs.h"

class ZDvidLabelSlice;
class ZFlyEmProofDoc;
class ZIntCuboid;
class ZFlyEmBookmark;
class ZDvidGraySlice;
class ZObject3dScan;

class ZFlyEmProofDocUtil
{
public:
  ZFlyEmProofDocUtil();

public:
  static ZDvidLabelSlice* GetActiveLabelSlice(ZFlyEmProofDoc *doc);
//  static ZDvidLabelSlice* GetActiveLabelSlice(ZFlyEmProofDoc *doc);
  static ZDvidGraySlice* GetActiveGraySlice(ZFlyEmProofDoc *doc);

  static std::set<uint64_t> GetSelectedBodyId(ZFlyEmProofDoc *doc, neutu::ELabelSource type);
  static void ExportSelectedBodyStack(ZFlyEmProofDoc *doc, bool isSparse,
      bool isFullRange, const ZIntCuboid &box, const QString &filePath);
  static void ExportSelecteBodyLevel(
      ZFlyEmProofDoc *doc, const ZIntCuboid &range,
      const QString &filePath);
  static void ExportSelectedBody(ZFlyEmProofDoc *doc, const QString &filePath);

  static bool HasSupervoxel(ZFlyEmProofDoc *doc);
  static bool HasSynapse(ZFlyEmProofDoc *doc);
  static bool HasWrittableSynapse(ZFlyEmProofDoc *doc);

  static QList<ZFlyEmBookmark*> GetUserBookmarkList(ZFlyEmProofDoc *doc);
};

#endif // ZFLYEMPROOFDOCUTIL_H
