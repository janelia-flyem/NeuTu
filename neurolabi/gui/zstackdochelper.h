#ifndef ZSTACKDOCHELPER_H
#define ZSTACKDOCHELPER_H

#include"zintpoint.h"

class ZStackDoc;
class ZIntCuboid;
class ZStack;
class ZFlyEmProofDoc;
class QColor;

class ZStackDocHelper
{
public:
  ZStackDocHelper();
  ~ZStackDocHelper();

  void extractCurrentZ(const ZStackDoc *doc);

  int getCurrentZ() const;
  bool hasCurrentZ() const;

  ZStack *getSparseStack(const ZStackDoc *doc);
  ZIntPoint getSparseStackDsIntv() const {
    return m_sparseStackDsIntv;
  }

  static ZIntCuboid getVolumeBoundBox(const ZStackDoc *doc);
  static QColor GetBodyColor(const ZFlyEmProofDoc *doc, uint64_t bodyId);

private:
  int m_currentZ;
  bool m_hasCurrentZ;
  ZStack *m_sparseStack;
  ZIntPoint m_sparseStackDsIntv;
};

#endif // ZSTACKDOCHELPER_H
