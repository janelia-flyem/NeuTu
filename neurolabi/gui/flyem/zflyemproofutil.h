#ifndef ZFLYEMPROOFUTIL_H
#define ZFLYEMPROOFUTIL_H

#include <functional>

#include <QWidget>

class ZFlyEmProofDoc;
class ZFlyEmBodyAnnotation;
class ZJsonObject;

class ZFlyEmProofUtil
{
public:
  ZFlyEmProofUtil();

public:
  static bool AnnotateBody(
      uint64_t bodyId, const ZFlyEmBodyAnnotation &annotation,
      const ZFlyEmBodyAnnotation &oldAnnotation,
      ZFlyEmProofDoc *doc, QWidget *parentWidget);

  static bool AnnotateBody(
      uint64_t bodyId, const ZJsonObject &annotation,
      const ZJsonObject &oldAnnotation,
      ZFlyEmProofDoc *doc, QWidget *parentWidget);      
};

#endif // ZFLYEMPROOFUTIL_H
