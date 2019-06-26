#ifndef ZFLYEMPROOFDOCUTIL_H
#define ZFLYEMPROOFDOCUTIL_H

#include <set>
#include <QString>
#include <QList>

#include "common/neutube_def.h"

class ZDvidLabelSlice;
class ZFlyEmProofDoc;
class ZIntCuboid;
class ZFlyEmBookmark;

class ZFlyEmProofDocUtil
{
public:
  ZFlyEmProofDocUtil();

public:
  static ZDvidLabelSlice* GetActiveLabelSlice(ZFlyEmProofDoc *doc, neutu::EAxis axis);
  static ZDvidLabelSlice* GetActiveLabelSlice(ZFlyEmProofDoc *doc);
  static std::set<uint64_t> GetSelectedBodyId(
      ZFlyEmProofDoc *doc,
      neutu::EAxis axis, neutu::ELabelSource type);
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
