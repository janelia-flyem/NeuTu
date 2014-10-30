#ifndef ZSTACKOBJECTSOURCE_H
#define ZSTACKOBJECTSOURCE_H

#include <string>

class ZStackObjectSource
{
public:
  ZStackObjectSource();

  enum ESpecialId {
    ID_BODY_GRAYSCALE_PATCH, ID_LOCAL_WATERSHED_BORDER, ID_RECT_ROI
  };

  static std::string getSource(ESpecialId id);
};

#endif // ZSTACKOBJECTSOURCE_H
