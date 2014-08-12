#ifndef ZJSONFACTORY_H
#define ZJSONFACTORY_H

class ZJsonArray;
class ZJsonObject;
class ZObject3dScan;

class ZJsonFactory
{
public:
  ZJsonFactory();

  static ZJsonArray makeJsonArray(const ZObject3dScan &obj);
};

#endif // ZJSONFACTORY_H
