#ifndef ZSTACKDOCUTIL_H
#define ZSTACKDOCUTIL_H

#include <string>

#include "common/neutudefs.h"

class ZStackDoc;
class ZIntCuboid;

class ZStackDocUtil
{
public:
  ZStackDocUtil();

public:
  static std::string SaveStack(const ZStackDoc *doc, const std::string &path);

  /*!
   * \brief Get the range of the current axis-shifted stack space.
   */
  static ZIntCuboid GetStackSpaceRange(
      const ZStackDoc *doc, neutu::EAxis sliceAxis);
  static ZIntCuboid GetStackSpaceRange(
      const ZStackDoc &doc, neutu::EAxis sliceAxis);

  static ZIntCuboid GetDataSpaceRange(const ZStackDoc &doc);
  static ZIntCuboid GetDataSpaceRange(const ZStackDoc *doc);
};

#endif // ZSTACKDOCUTIL_H
