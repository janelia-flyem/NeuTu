#include "zstackobjectsource.h"

ZStackObjectSource::ZStackObjectSource()
{
}

std::string ZStackObjectSource::getSource(ESpecialId id)
{
  switch (id) {
  case ID_BODY_GRAYSCALE_PATCH:
    return "#grayscale_patch";
  case ID_LOCAL_WATERSHED_BORDER:
    return "#localSeededWatershed:Temporary_Border:";
  case ID_RECT_ROI:
    return "#rect_roi";
  }

  return "";
}
