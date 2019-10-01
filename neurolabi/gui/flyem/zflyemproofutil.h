#ifndef ZFLYEMPROOFUTIL_H
#define ZFLYEMPROOFUTIL_H

#include <QWidget>

class ZFlyEmProofDoc;
class ZFlyEmBodyAnnotation;

class ZFlyEmProofUtil
{
public:
  ZFlyEmProofUtil();

public:
  static bool AnnotateBody(
      uint64_t bodyId, const ZFlyEmBodyAnnotation &annotation,
      const ZFlyEmBodyAnnotation &oldAnnotation,
      ZFlyEmProofDoc *doc, QWidget *parentWidget);
};

#endif // ZFLYEMPROOFUTIL_H
