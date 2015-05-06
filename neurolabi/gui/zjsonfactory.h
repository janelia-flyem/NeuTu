#ifndef ZJSONFACTORY_H
#define ZJSONFACTORY_H

#include <QMap>
#include "tz_stdint.h"

class ZJsonArray;
class ZJsonObject;
class ZObject3dScan;

class ZJsonFactory
{
public:
  ZJsonFactory();

  enum EObjectForm {
    OBJECT_DENSE, OBJECT_SPARSE
  };

  static ZJsonArray MakeJsonArray(const ZObject3dScan &obj,
                                  EObjectForm form = OBJECT_SPARSE);

  static ZJsonArray MakeJsonArray(const QMap<uint64_t, uint64_t> &map);
};

#endif // ZJSONFACTORY_H
