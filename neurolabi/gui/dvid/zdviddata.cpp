#include "zdviddata.h"

const char* ZDvidData::m_grayScaleName = "grayscale";
const char* ZDvidData::m_bodyLabelName = "bodies";
const char* ZDvidData::m_roiCurveName = "roi_curve";
const char* ZDvidData::m_bodyAnnotationName = "annotations";
const char* ZDvidData::m_boundBoxName = "bound_box";
const char* ZDvidData::m_skeletonName = "skeletons";
const char* ZDvidData::m_thumbnailName = "thumbnails";
const char* ZDvidData::m_superpixelName = "superpixels";
const char* ZDvidData::m_sp2bodyName = "sp2body";
const char* ZDvidData::m_sparsevolName = "sparsevol";
const char* ZDvidData::m_coarseSparsevolName = "sparsevol-coarse";
const char* ZDvidData::m_splitLabelName = "splits";
const char* ZDvidData::m_bodyInfoName = "bodyinfo";
const char* ZDvidData::m_mergeTestBodyLabelName = "merge_test";
const char* ZDvidData::m_maxBodyIdName = "max_body_id";
const char* ZDvidData::m_splitStatusName = "split_status";
const char* ZDvidData::m_labelBlockName = "labels";
const char* ZDvidData::m_multiscale2dName = "tiles";
const char* ZDvidData::m_mergeOperationName = "neutu_merge_opr";
const char* ZDvidData::m_bookmarkKeyName = "bookmarks";
const char* ZDvidData::m_bookmarkAnnotationName = "bookmark_annotations";
const char* ZDvidData::m_bodySynapsesName = "body_synapses";
const char* ZDvidData::m_todoListName = "todo";
const char* ZDvidData::m_synapseName = ""; //No default

//const char* ZDvidData::m_keyValueTypeName = "keyvalue";

const char* ZDvidData::m_emptyName = "";

ZDvidData::ZDvidData()
{
}

const char* ZDvidData::GetName(ERole role)
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
    /*
  case ROLE_SPARSEVOL:
    return m_sparsevolName;
  case ROLE_SPARSEVOL_COARSE:
    return m_coarseSparsevolName;
    */
  case ROLE_SPLIT_LABEL:
    return m_splitLabelName;
  case ROLE_SPLIT_STATUS:
    return m_splitStatusName;
  case ROLE_BODY_INFO:
    return m_bodyInfoName;
  case ROLE_MERGE_TEST_BODY_LABEL:
    return m_mergeTestBodyLabelName;
  case ROLE_MAX_BODY_ID:
    return m_maxBodyIdName;
  case ROLE_LABEL_BLOCK:
    return m_labelBlockName;
  case ROLE_MULTISCALE_2D:
    return m_multiscale2dName;
  case ROLE_MERGE_OPERATION:
    return m_mergeOperationName;
  case ROLE_BOOKMARK_KEY:
    return m_bookmarkKeyName;
  case ROLE_BOOKMARK:
    return m_bookmarkAnnotationName;
  case ROLE_BODY_SYNAPSES:
    return m_bodySynapsesName;
  case ROLE_SYNAPSE:
    return m_synapseName;
  case ROLE_TODO_LIST:
    return m_todoListName;
  }

  return m_emptyName;
}

/*
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
*/
std::string ZDvidData::GetName(ERole role, const std::string &prefix)
{
  if (prefix.empty()) {
    return ZDvidData::GetName(role);
  }

  return prefix + "_" + ZDvidData::GetName(role);
}

std::string ZDvidData::GetName(
    ZDvidData::ERole role, ZDvidData::ERole prefixRole,
    const std::string &prefixName)
{

  std::string prefix = "";
  if (!isDefaultName(prefixRole, prefixName)) {
    prefix = prefixName;
  }

  return GetName(role, prefix);
}

bool ZDvidData::isDefaultName(ERole role, const std::string &name)
{
  /*
  if (role == ZDvidData::ROLE_BODY_LABEL) {
    if (name == m_sp2bodyName) {
      return true;
    }
  }
  */

  return ZDvidData::GetName(role) == name;
}
