#include "zfiletype.h"

//#include <QString>
#include "zstring.h"

#ifdef _QT_GUI_USED_
#include "zmesh.h"
#endif

ZFileType::ZFileType()
{
}

ZFileType::EFileType ZFileType::FileType(const std::string &filePath)
{
  ZString str(filePath.c_str());

  if (str.endsWith(".swc", ZString::CASE_INSENSITIVE)) {
    return EFileType::SWC;
  } else if (str.endsWith(".tif", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".tiff", ZString::CASE_INSENSITIVE)) {
    return EFileType::TIFF;
  } else if (str.endsWith(".lsm", ZString::CASE_INSENSITIVE)) {
    return EFileType::LSM;
  } else if (str.endsWith(".png", ZString::CASE_INSENSITIVE)) {
    return EFileType::PNG;
  } else if (str.endsWith(".tb", ZString::CASE_INSENSITIVE)) {
    return EFileType::LOCSEG_CHAIN;
  } else if (str.endsWith(".nnt", ZString::CASE_INSENSITIVE)) {
    return EFileType::SWC_NETWORK;
  } else if (str.endsWith(".json", ZString::CASE_INSENSITIVE)) {
    return EFileType::JSON;
  } else if (str.endsWith(".fnt", ZString::CASE_INSENSITIVE) ) {
    return EFileType::FLYEM_NETWORK;
  } else if (str.endsWith(".xml", ZString::CASE_INSENSITIVE)) {
    return EFileType::XML;
  } else if (str.endsWith(".apo", ZString::CASE_INSENSITIVE)) {
    return EFileType::V3D_APO;
  } else if (str.endsWith(".marker", ZString::CASE_INSENSITIVE)) {
    return EFileType::V3D_MARKER;
  } else if (str.endsWith(".raw", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".v3draw", ZString::CASE_INSENSITIVE)) {
    return EFileType::V3D_RAW;
  } else if (str.endsWith(".v3dpbd", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".pbd", ZString::CASE_INSENSITIVE)) {
    return EFileType::V3D_PBD;
  } else if (str.endsWith(".puncta", ZString::CASE_INSENSITIVE)) {
    return EFileType::PUNCTA;
  } else if (str.endsWith(".txt", ZString::CASE_INSENSITIVE)) {
    FILE *fp = fopen(str.c_str(), "r");
    if (fp != NULL) {
      ZString str;
      int patternCount = 0;
      while (str.readLine(fp)) {
        if (str.startsWith("session:")) {
          patternCount++;
        }
        if (patternCount == 1) {
          if (str.startsWith("bookmarks")) {
            patternCount++;
          }
        }
      }
      fclose(fp);

      if (patternCount >= 2) {
        return EFileType::RAVELER_BOOKMARK;
      }
    }
    return EFileType::TXT;
  } else if (str.endsWith(".nsp", ZString::CASE_INSENSITIVE)) {
    return EFileType::MYERS_NSP;
  } else if (str.endsWith(".sobj", ZString::CASE_INSENSITIVE)) {
    return EFileType::OBJECT_SCAN;
  } else if (str.endsWith(".soba", ZString::CASE_INSENSITIVE)) {
    return EFileType::OBJECT_SCAN_ARRAY;
  } else if (str.endsWith(".jpg", ZString::CASE_INSENSITIVE)) {
    return EFileType::JPG;
  } else if (str.endsWith(".dvid", ZString::CASE_INSENSITIVE)) {
    return EFileType::DVID_OBJECT;
  } else if (str.endsWith(".hf5", ZString::CASE_INSENSITIVE)) {
    return EFileType::HDF5;
  } else if (str.endsWith(".mraw", ZString::CASE_INSENSITIVE)) {
    return EFileType::MC_STACK_RAW;
  } else if (str.endsWith(".zss", ZString::CASE_INSENSITIVE)) {
    return EFileType::SPARSE_STACK;
  } else if (str.endsWith(".g3d", ZString::CASE_INSENSITIVE)) {
    return EFileType::GRAPH_3D;
  }
#if _QT_GUI_USED_
  else if (ZMesh::canReadFile(QString::fromStdString(filePath))) {
    return EFileType::MESH;
  }
#endif


  return EFileType::UNIDENTIFIED;
}

std::string ZFileType::TypeName(EFileType type)
{
  switch (type) {
  case EFileType::SWC:
    return "SWC";
  case EFileType::SWC_NETWORK:
    return "SWC Network";
  case EFileType::LOCSEG_CHAIN:
    return "Locseg Chain";
  case EFileType::TIFF:
    return "TIFF";
  case EFileType::LSM:
    return "LSM";
  case EFileType::PNG:
    return "PNG";
  case EFileType::V3D_RAW:
    return "Vaa3D raw";
  case EFileType::SYNAPSE_ANNOTATON:
    return "Synapse Annotation";
  case EFileType::FLYEM_NETWORK:
    return "FlyEM Network";
  case EFileType::XML:
    return "XML";
  case EFileType::JSON:
    return "Json";
  case EFileType::V3D_APO:
    return "Vaa3d APO";
  case EFileType::V3D_PBD:
    return "v3dpbd";
  case EFileType::V3D_MARKER:
    return "Vaa3d marker";
  case EFileType::RAVELER_BOOKMARK:
    return "Reveler bookmark";
  case EFileType::MYERS_NSP:
    return "Neuron segmentation";
  case EFileType::HDF5:
    return "HDF5";
  case EFileType::MESH:
    return "Mesh";
  case EFileType::PUNCTA:
    return "Puncta";
  default:
    return "Unknown";
  }
}

bool ZFileType::IsImageFile(EFileType type)
{
  return (type == EFileType::TIFF) ||
      (type == EFileType::LSM) ||
      (type == EFileType::PNG) ||
      (type == EFileType::V3D_RAW) ||
      (type == EFileType::V3D_PBD) ||
      (type == EFileType::MYERS_NSP) || /*(type == OBJECT_SCAN_FILE) ||*/
      (type == EFileType::JPG) ||
      (type == EFileType::DVID_OBJECT) ||
      (type == EFileType::MC_STACK_RAW);
}

bool ZFileType::IsImageFile(const std::string &filePath)
{
  return IsImageFile(FileType(filePath));
}

bool ZFileType::IsObjectFile(EFileType type)
{
  return (type == EFileType::SWC) ||
      (type == EFileType::SWC_NETWORK) ||
      (type == EFileType::LOCSEG_CHAIN) ||
      (type == EFileType::SYNAPSE_ANNOTATON) ||
      (type == EFileType::FLYEM_NETWORK) ||
      (type == EFileType::JSON) ||
      (type == EFileType::V3D_APO) ||
      (type == EFileType::V3D_MARKER) ||
      (type == EFileType::RAVELER_BOOKMARK) ||
      (type == EFileType::PUNCTA) ||
      (type == EFileType::OBJECT_SCAN) ||
      (type == EFileType::OBJECT_SCAN_ARRAY) ||
      (type == EFileType::SPARSE_STACK) ||
      (type == EFileType::MESH) ||
      (type == EFileType::GRAPH_3D);
}

bool ZFileType::IsObjectFile(const std::string &filePath)
{
  return IsObjectFile(FileType(filePath));
}

bool ZFileType::IsNeutubeOpenable(const std::string &filePath)
{
  return IsNeutubeOpenable(FileType(filePath));
}

bool ZFileType::IsNeutubeOpenable(EFileType type)
{
  bool openable = IsImageFile(type);
  if (!openable) {
    openable = IsObjectFile(type);
  }
  return openable;
}
