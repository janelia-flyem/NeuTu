#ifndef ZDVIDDATA_H
#define ZDVIDDATA_H

/*!
 * \brief The class of dvid data
 */
class ZDvidData
{
public:
  ZDvidData();

  enum ERole {
    ROLE_GRAY_SCALE, ROLE_BODY_LABEL, ROLE_ROI_CURVE, ROLE_BODY_ANNOTATION,
    ROLE_BOUND_BOX, ROLE_SKELETON, ROLE_THUMBNAIL, ROLE_SUPERPIXEL,
    ROLE_SP2BODY, ROLE_SPARSEVOL, ROLE_SPLIT_LABEL
  };

  enum EType {
    TYPE_LABEL_GRAPH, TYPE_ROI, TYPE_GRAYSCALE8, TYPE_RGBA8, TYPE_LABLES64,
    TYPE_LABELMAP, TYPE_MULTISCALE_2D, TYPE_MULTCHAN16, TYPE_KEY_VALUE
  };

  static const char* getName(ERole role);
  static const char* getName(EType type);

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
  static const char *m_splitLabelName;

  static const char *m_keyValueTypeName;

  static const char *m_emptyName;
};

#endif // ZDVIDDATA_H
