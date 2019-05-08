#ifndef ZFILETYPE_H
#define ZFILETYPE_H

#include <string>

class ZFileType
{
public:
  ZFileType();

  enum class EFileType {
    UNIDENTIFIED, SWC, SWC_NETWORK,
    LOCSEG_CHAIN, TIFF, LSM, PNG, V3D_RAW,
    SYNAPSE_ANNOTATON, FLYEM_NETWORK, XML, JSON,
    V3D_APO, V3D_MARKER,
    RAVELER_BOOKMARK, V3D_PBD, PUNCTA,
    MYERS_NSP, OBJECT_SCAN, OBJECT_SCAN_ARRAY,
    JPG, DVID_OBJECT, HDF5,
    MC_STACK_RAW, TXT, SPARSE_STACK,
    MESH, GRAPH_3D
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
