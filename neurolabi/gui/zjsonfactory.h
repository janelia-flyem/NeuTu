#ifndef ZJSONFACTORY_H
#define ZJSONFACTORY_H

class ZJsonArray;
class ZJsonObject;

class ZJsonFactory
{
public:
  ZJsonFactory();

  static ZJsonArray makeJsonArray(const ZJsonObject &obj);
};

#endif // ZJSONFACTORY_H
