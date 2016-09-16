#ifndef ZDVIDDATA_H
#define ZDVIDDATA_H

#include <string>

/*!
 * \brief The class of dvid data
 *
 * An instance of DVID data has a type and a name. It can also have role in the
 * framework of FlyEM reconstruction. This class has built-in correspondence
 * between a role and a name, providing the default name of a given role.
 */
class ZDvidData
{
public:
  ZDvidData();

  enum ERole {
    ROLE_GRAY_SCALE, //Grayscale data
    ROLE_BODY_LABEL, //Individual body data (sparsevol)
    ROLE_LABEL_BLOCK, //Segmentation data
    ROLE_ROI_CURVE, //ROI curve
    ROLE_BODY_ANNOTATION, //Body Annnotation
    ROLE_BOUND_BOX, //Bounding boxes of bodies
    ROLE_SKELETON, //Skeleton
    ROLE_THUMBNAIL, //Thumbnail
    ROLE_SUPERPIXEL, //Superpixel (NOT in DVID)
    ROLE_SP2BODY, // Superpixel mapping (NOT in DVID)
    /*ROLE_SPARSEVOL, ROLE_SPARSEVOL_COARSE,*/
    ROLE_SPLIT_LABEL, //Obsolete
    ROLE_SPLIT_STATUS, //Obsolete
    ROLE_BODY_INFO, //Body information, e.g. size and bounding box
    ROLE_MERGE_TEST_BODY_LABEL, //Obsolete
    ROLE_MAX_BODY_ID, //Maximal body ID
    ROLE_MULTISCALE_2D, //Obsolete
    ROLE_MERGE_OPERATION, //Merge operation
    ROLE_BOOKMARK, //Bookmark
    ROLE_BOOKMARK_KEY,
    ROLE_BODY_SYNAPSES,
    ROLE_LABELSZ,
    ROLE_SYNAPSE,
    ROLE_TODO_LIST,
    ROLE_NEUTU_CONFIG
  };

  enum EType {
    TYPE_LABEL_GRAPH, TYPE_ROI, TYPE_GRAYSCALE8, TYPE_RGBA8, TYPE_LABLES64,
    TYPE_LABELMAP, TYPE_MULTISCALE_2D, TYPE_MULTCHAN16, TYPE_KEY_VALUE
  };

  static const char* GetName(ERole role);
  //static const char* getName(EType type);

  static std::string GetName(
      ZDvidData::ERole role, ZDvidData::ERole prefixRole,
      const std::string &prefixName);
//  static std::string GetName(ERole role, ERole prefixRole);

  static bool isDefaultName(ERole role, const std::string &name);
  static bool IsNullName(const std::string &name);

private:
  static std::string GetName(ERole role, const std::string &prefix);

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
  //static const char *m_keyValueTypeName;

  static const char *m_nullName;
  static const char *m_emptyName;
};

#endif // ZDVIDDATA_H
