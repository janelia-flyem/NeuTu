#ifndef ZJSONFACTORY_H
#define ZJSONFACTORY_H

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

  static ZJsonArray makeJsonArray(const ZObject3dScan &obj,
                                  EObjectForm form = OBJECT_SPARSE);
};

#endif // ZJSONFACTORY_H
