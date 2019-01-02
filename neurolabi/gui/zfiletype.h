#ifndef ZFILETYPE_H
#define ZFILETYPE_H

#include <string>

class ZFileType
{
public:
  ZFileType();

  enum EFileType {
    FILE_UNIDENTIFIED, FILE_SWC, FILE_SWC_NETWORK,
    FILE_LOCSEG_CHAIN, FILE_TIFF, FILE_LSM, FILE_PNG, FILE_V3D_RAW,
    FILE_SYNAPSE_ANNOTATON, FILE_FLYEM_NETWORK, FILE_XML, FILE_JSON,
    FILE_V3D_APO, FILE_V3D_MARKER,
    FILE_RAVELER_BOOKMARK, FILE_V3D_PBD, FILE_MYERS_NSP, FILE_OBJECT_SCAN,
    FILE_OBJECT_SCAN_ARRAY,
    FILE_JPG, FILE_DVID_OBJECT, FILE_HDF5,
    FILE_MC_STACK_RAW, FILE_TXT, FILE_SPARSE_STACK,
    FILE_MESH, FILE_3D_GRAPH
  };

  static EFileType FileType(const std::string &filePath);
  static std::string TypeName(EFileType type);

  static bool isImageFile(const std::string &filePath);
  static bool isObjectFile(const std::string &filePath);
  static bool isImageFile(EFileType type);
  static bool isObjectFile(EFileType type);
  static bool isNeutubeOpenable(EFileType type);
  static bool isNeutubeOpenable(const std::string &filePath);
};

#endif // ZFILETYPE_H
