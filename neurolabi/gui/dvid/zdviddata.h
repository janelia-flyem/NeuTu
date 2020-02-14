#ifndef ZDVIDDATA_H
#define ZDVIDDATA_H

#include <string>

/*!
 * \brief The class of dvid data
 *
 * An instance of DVID data has a type and a name. It can also have a role in the
 * framework of FlyEM reconstruction. This class has built-in correspondence
 * between a role and a name, providing the default name of a given role.
 */
class ZDvidData
{
public:
  ZDvidData();

  enum class ERole {
    GRAYSCALE, //Grayscale data
    SEGMENTATION, //Segmentation data
    SPARSEVOL, //Individual body data (sparsevol)
    ROI_CURVE, //ROI curve
    BODY_ANNOTATION, //Body Annnotation
    BOUND_BOX, //Bounding boxes of bodies
    SKELETON, //Skeleton
    THUMBNAIL, //Thumbnail
    SUPERPIXEL, //Superpixel (NOT in DVID)
    SP2BODY, // Superpixel mapping (NOT in DVID)
    /*ROLE_SPARSEVOL, ROLE_SPARSEVOL_COARSE,*/
    SPLIT_LABEL,
    SPLIT_STATUS, //Obsolete
    BODY_INFO, //Body information, e.g. size and bounding box
    MERGE_TEST_BODY_LABEL, //Obsolete
    MAX_BODY_ID, //Maximal body ID, Obsolete
    MULTISCALE_2D, //Obsolete
    MERGE_OPERATION, //Merge operation
    BOOKMARK, //Bookmark
    BOOKMARK_KEY,
    BODY_SYNAPSES,
    LABELSZ,
    SYNAPSE,
    TODO_LIST,
    SPARSEVOL_SIZE,
    NEUTU_CONFIG,
    RESULT_KEY,
    TASK_KEY,
    SPLIT_GROUP,
    SPLIT_TASK_KEY,
    SPLIT_RESULT_KEY,
    SPLIT_TASK_PROPERTY_KEY,
    SPLIT_RESULT_PROPERTY_KEY,
    MESH,
    MESHES_TARS,
    TAR_SUPERVOXELS,
    ROI_KEY,
    ROI_DATA_KEY,
    TEST_TASK_KEY,
    TEST_RESULT_KEY
  };

  enum class EType {
    LABEL_GRAPH, ROI, GRAYSCALE8, RGBA8, LABLES64,
    LABELBLK, LABELARRAY, LABELMAP, MULTISCALE_2D,
    MULTCHAN16, KEY_VALUE
  };

  static std::string GetName(ERole role);

  template<typename T>
  static T GetName(ERole role);
  //static const char* getName(EType type);

  static std::string GetName(
      ZDvidData::ERole role, ZDvidData::ERole prefixRole,
      const std::string &prefixName);
  static std::string GetName(ERole role, const std::string &prefix);
//  static std::string GetName(ERole role, ERole prefixRole);

  static bool IsNullName(const std::string &name);

  static std::string GetTaskName(const std::string &group);
  static std::string GetResultName(const std::string &group);

  template<typename T>
  static T GetTaskName(const std::string &group);

  template<typename T>
  static T GetResultName(const std::string &group);


private:
  static bool IsDefaultName(ERole role, const std::string &name);

private:
  static const char *m_grayScaleName;
  static const char *m_bodyLabelName;
  static const char *m_roiCurveName;
  static const char *m_bodyAnnotationName;
  static const char *m_boundBoxName;
  static const char *m_skeletonName;
  static const char *m_thumbnailName;
  static const char *m_superpixelName;
  static const char *m_sp2bodyName;
  static const char *m_sparsevolName;
  static const char *m_coarseSparsevolName;
  static const char *m_splitLabelName;
  static const char *m_splitStatusName;
  static const char *m_bodyInfoName;
  static const char *m_mergeTestBodyLabelName;
  static const char *m_maxBodyIdName;
  static const char *m_labelBlockName;
  static const char *m_multiscale2dName;
  static const char *m_mergeOperationName;
  static const char *m_bookmarkKeyName;
  static const char *m_bookmarkAnnotationName;
  static const char *m_bodySynapsesName;
  static const char *m_synapseName;
  static const char *m_todoListName;
  static const char *m_neutuConfigName;
  static const char *m_labelszName;
  static const char *m_meshName;
  static const char *m_sparsevolSizeName;
  //static const char *m_keyValueTypeName;
  static const char *m_meshesTarsName;
  static const char *m_tarSupervoxelsName;
  static const char *m_roiKeyName;
  static const char *m_roiDataKeyName;
  static const char *m_testTaskKeyName;
  static const char *m_testResultKeyName;

  static const char *m_nullName;
  static const char *m_emptyName;
};

template<typename T>
T ZDvidData::GetName(ERole role)
{
  return T(GetName(role).c_str());
}

template<typename T>
T ZDvidData::GetResultName(const std::string &group)
{
  return T(GetResultName(group).c_str());
}

template<typename T>
T ZDvidData::GetTaskName(const std::string &group)
{
  return T(GetTaskName(group).c_str());
}

#endif // ZDVIDDATA_H
