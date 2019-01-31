#include "zdviddata.h"

const char* ZDvidData::m_grayScaleName = "grayscale";
const char* ZDvidData::m_bodyLabelName = "";
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
const char* ZDvidData::m_labelBlockName = "";
const char* ZDvidData::m_multiscale2dName = "tiles";
const char* ZDvidData::m_mergeOperationName = "neutu_merge_opr";
const char* ZDvidData::m_bookmarkKeyName = "bookmarks";
const char* ZDvidData::m_bookmarkAnnotationName = "bookmark_annotations";
const char* ZDvidData::m_bodySynapsesName = "body_synapses";
const char* ZDvidData::m_todoListName = "todo";
const char* ZDvidData::m_synapseName = ""; //No default
const char* ZDvidData::m_neutuConfigName = "neutu_config";
const char* ZDvidData::m_labelszName = "labelsz";
const char* ZDvidData::m_meshName = "meshes";
const char* ZDvidData::m_sparsevolSizeName = "sparsevol-size";
const char* ZDvidData::m_meshesTarsName = "meshes_tars";
const char* ZDvidData::m_tarSupervoxelsName = "sv_meshes";
const char* ZDvidData::m_roiKeyName = "rois";
const char* ZDvidData::m_roiDataKeyName = "roi_data";
const char* ZDvidData::m_testTaskKeyName = "task_test";
const char* ZDvidData::m_testResultKeyName = "result_test";


//const char* ZDvidData::m_keyValueTypeName = "keyvalue";

const char* ZDvidData::m_nullName = "*";
const char* ZDvidData::m_emptyName = "";

ZDvidData::ZDvidData()
{
}

std::string ZDvidData::GetName(ERole role)
{
  switch (role) {
  case ERole::GRAY_SCALE:
    return m_grayScaleName;
  case ERole::BODY_LABEL:
    return m_bodyLabelName;
  case ERole::ROI_CURVE:
    return m_roiCurveName;
  case ERole::BODY_ANNOTATION:
    return m_bodyAnnotationName;
  case ERole::BOUND_BOX:
    return m_boundBoxName;
  case ERole::SKELETON:
    return m_skeletonName;
  case ERole::THUMBNAIL:
    return m_thumbnailName;
  case ERole::SUPERPIXEL:
    return m_superpixelName;
  case ERole::SP2BODY:
    return m_sp2bodyName;
    /*
  case ROLE_SPARSEVOL:
    return m_sparsevolName;
  case ROLE_SPARSEVOL_COARSE:
    return m_coarseSparsevolName;
    */
  case ERole::SPLIT_LABEL:
    return m_splitLabelName;
  case ERole::SPLIT_STATUS:
    return m_splitStatusName;
  case ERole::BODY_INFO:
    return m_bodyInfoName;
  case ERole::MERGE_TEST_BODY_LABEL:
    return m_mergeTestBodyLabelName;
  case ERole::MAX_BODY_ID:
    return m_maxBodyIdName;
  case ERole::LABEL_BLOCK:
    return m_labelBlockName;
  case ERole::MULTISCALE_2D:
    return m_multiscale2dName;
  case ERole::MERGE_OPERATION:
    return m_mergeOperationName;
  case ERole::BOOKMARK_KEY:
    return m_bookmarkKeyName;
  case ERole::BOOKMARK:
    return m_bookmarkAnnotationName;
  case ERole::BODY_SYNAPSES:
    return m_bodySynapsesName;
  case ERole::SYNAPSE:
    return m_synapseName;
  case ERole::TODO_LIST:
    return m_todoListName;
  case ERole::LABELSZ:
    return m_labelszName;
  case ERole::SPARSEVOL_SIZE:
    return m_sparsevolSizeName;
  case ERole::NEUTU_CONFIG:
    return m_neutuConfigName;
  case ERole::RESULT_KEY:
    return "result";
  case ERole::TASK_KEY:
    return "task";
  case ERole::SPLIT_GROUP:
    return "split";
  case ERole::SPLIT_RESULT_KEY:
    return GetName(ERole::RESULT_KEY) + "_" + GetName(ERole::SPLIT_GROUP);
  case ERole::SPLIT_TASK_KEY:
    return GetName(ERole::TASK_KEY) + "_" + GetName(ERole::SPLIT_GROUP);
  case ERole::SPLIT_RESULT_PROPERTY_KEY:
    return GetName(ERole::SPLIT_RESULT_KEY) + "_" + "property";
  case ERole::SPLIT_TASK_PROPERTY_KEY:
    return GetName(ERole::SPLIT_TASK_KEY) + "_" + "property";
  case ERole::MESH:
    return m_meshName;
  case ERole::MESHES_TARS:
    return m_meshesTarsName;
  case ERole::TAR_SUPERVOXELS:
    return m_tarSupervoxelsName;
  case ERole::ROI_KEY:
    return m_roiKeyName;
  case ERole::ROI_DATA_KEY:
    return m_roiDataKeyName;
  case ERole::TEST_TASK_KEY:
    return m_testTaskKeyName;
  case ERole::TEST_RESULT_KEY:
    return m_testResultKeyName;
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
  if (IsNullName(prefix) || prefix.empty()) {
    return "";
  }

  /*
  if (prefix.empty()) {
    return ZDvidData::GetName(role);
  }
  */

  return prefix + "_" + ZDvidData::GetName(role);
}

std::string ZDvidData::GetName(
    ZDvidData::ERole role, ZDvidData::ERole prefixRole,
    const std::string &prefixName)
{

  std::string prefix = "";
  if (!IsDefaultName(prefixRole, prefixName)) {
    prefix = prefixName;
  } else {
    return ZDvidData::GetName(role);
  }

  return GetName(role, prefix);
}

std::string ZDvidData::GetResultName(const std::string &group)
{
  return GetName(ERole::RESULT_KEY) + "_" + group;
}

std::string ZDvidData::GetTaskName(const std::string &group)
{
  return GetName(ERole::TASK_KEY) + "_" + group;
}

bool ZDvidData::IsDefaultName(ERole role, const std::string &name)
{
  if (name.empty()) {
    return false;
  }

  if (role == ZDvidData::ERole::BODY_LABEL) { //For backfward compability
    if (name == "bodies") {
      return true;
    }
  }

  if (role == ZDvidData::ERole::LABEL_BLOCK) {
    if (name == "labels") {
      return true;
    }
  }

  return ZDvidData::GetName(role) == name;
}

bool ZDvidData::IsNullName(const std::string &name)
{
  return name == m_nullName;
}
