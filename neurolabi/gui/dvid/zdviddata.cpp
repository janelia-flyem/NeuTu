#include "zdviddata.h"

const char* ZDvidData::m_grayScaleName = "grayscale";
const char* ZDvidData::m_bodyLabelName = "bodylabel";
const char* ZDvidData::m_roiCurveName = "roi_curve";
const char* ZDvidData::m_bodyAnnotationName = "annotations";
const char* ZDvidData::m_boundBoxName = "bound_box";
const char* ZDvidData::m_skeletonName = "skeletons";
const char* ZDvidData::m_thumbnailName = "thumbnails";
const char* ZDvidData::m_superpixelName = "superpixels";
const char* ZDvidData::m_sp2bodyName = "sp2body";
const char* ZDvidData::m_sparsevolName = "sparsevol";

const char* ZDvidData::m_keyValueTypeName = "keyvalue";

const char* ZDvidData::m_emptyName = "";

ZDvidData::ZDvidData()
{
}

const char* ZDvidData::getName(ERole role)
{
  switch (role) {
  case ROLE_GRAY_SCALE:
    return m_grayScaleName;
  case ROLE_BODY_LABEL:
    return m_bodyLabelName;
  case ROLE_ROI_CURVE:
    return m_roiCurveName;
  case ROLE_BODY_ANNOTATION:
    return m_bodyAnnotationName;
  case ROLE_BOUND_BOX:
    return m_boundBoxName;
  case ROLE_SKELETON:
    return m_skeletonName;
  case ROLE_THUMBNAIL:
    return m_thumbnailName;
  case ROLE_SUPERPIXEL:
    return m_superpixelName;
  case ROLE_SP2BODY:
    return m_sp2bodyName;
  case ROLE_SPARSEVOL:
    return m_sparsevolName;
  }

  return m_emptyName;
}

const char* ZDvidData::getName(EType type)
{
  switch (type) {
  case TYPE_KEY_VALUE:
    return m_keyValueTypeName;
  default:
    break;
  }

  return m_emptyName;
}
