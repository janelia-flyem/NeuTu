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
    return FILE_SWC;
  } else if (str.endsWith(".tif", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".tiff", ZString::CASE_INSENSITIVE)) {
    return FILE_TIFF;
  } else if (str.endsWith(".lsm", ZString::CASE_INSENSITIVE)) {
    return FILE_LSM;
  } else if (str.endsWith(".png", ZString::CASE_INSENSITIVE)) {
    return FILE_PNG;
  } else if (str.endsWith(".tb", ZString::CASE_INSENSITIVE)) {
    return FILE_LOCSEG_CHAIN;
  } else if (str.endsWith(".nnt", ZString::CASE_INSENSITIVE)) {
    return FILE_SWC_NETWORK;
  } else if (str.endsWith(".json", ZString::CASE_INSENSITIVE)) {
    return FILE_JSON;
  } else if (str.endsWith(".fnt", ZString::CASE_INSENSITIVE) ) {
    return FILE_FLYEM_NETWORK;
  } else if (str.endsWith(".xml", ZString::CASE_INSENSITIVE)) {
    return FILE_XML;
  } else if (str.endsWith(".apo", ZString::CASE_INSENSITIVE)) {
    return FILE_V3D_APO;
  } else if (str.endsWith(".marker", ZString::CASE_INSENSITIVE)) {
    return FILE_V3D_MARKER;
  } else if (str.endsWith(".raw", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".v3draw", ZString::CASE_INSENSITIVE)) {
    return FILE_V3D_RAW;
  } else if (str.endsWith(".v3dpbd", ZString::CASE_INSENSITIVE) ||
             str.endsWith(".pbd", ZString::CASE_INSENSITIVE)) {
    return FILE_V3D_PBD;
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
        return FILE_RAVELER_BOOKMARK;
      }
    }
    return FILE_TXT;
  } else if (str.endsWith(".nsp", ZString::CASE_INSENSITIVE)) {
    return FILE_MYERS_NSP;
  } else if (str.endsWith(".sobj", ZString::CASE_INSENSITIVE)) {
    return FILE_OBJECT_SCAN;
  } else if (str.endsWith(".soba", ZString::CASE_INSENSITIVE)) {
    return FILE_OBJECT_SCAN_ARRAY;
  } else if (str.endsWith(".jpg", ZString::CASE_INSENSITIVE)) {
    return FILE_JPG;
  } else if (str.endsWith(".dvid", ZString::CASE_INSENSITIVE)) {
    return FILE_DVID_OBJECT;
  } else if (str.endsWith(".hf5", ZString::CASE_INSENSITIVE)) {
    return FILE_HDF5;
  } else if (str.endsWith(".mraw", ZString::CASE_INSENSITIVE)) {
    return FILE_MC_STACK_RAW;
  } else if (str.endsWith(".zss", ZString::CASE_INSENSITIVE)) {
    return FILE_SPARSE_STACK;
  } else if (str.endsWith(".g3d", ZString::CASE_INSENSITIVE)) {
    return FILE_3D_GRAPH;
  }
#if _QT_GUI_USED_
  else if (ZMesh::canReadFile(QString::fromStdString(filePath))) {
    return FILE_MESH;
  }
#endif


  return FILE_UNIDENTIFIED;
}

std::string ZFileType::TypeName(EFileType type)
{
  switch (type) {
  case FILE_SWC:
    return "SWC";
  case FILE_SWC_NETWORK:
    return "SWC Network";
  case FILE_LOCSEG_CHAIN:
    return "Locseg Chain";
  case FILE_TIFF:
    return "TIFF";
  case FILE_LSM:
    return "LSM";
  case FILE_PNG:
    return "PNG";
  case FILE_V3D_RAW:
    return "Vaa3D raw";
  case FILE_SYNAPSE_ANNOTATON:
    return "Synapse Annotation";
  case FILE_FLYEM_NETWORK:
    return "FlyEM Network";
  case FILE_XML:
    return "XML";
  case FILE_JSON:
    return "Json";
  case FILE_V3D_APO:
    return "Vaa3d APO";
  case FILE_V3D_PBD:
    return "v3dpbd";
  case FILE_V3D_MARKER:
    return "Vaa3d marker";
  case FILE_RAVELER_BOOKMARK:
    return "Reveler bookmark";
  case FILE_MYERS_NSP:
    return "Neuron segmentation";
  case FILE_HDF5:
    return "HDF5";
  case FILE_MESH:
    return "Mesh";
  default:
    return "Unknown";
  }
}

bool ZFileType::isImageFile(EFileType type)
{
  return (type == FILE_TIFF) ||
      (type == FILE_LSM) ||
      (type == FILE_PNG) ||
      (type == FILE_V3D_RAW) ||
      (type == FILE_V3D_PBD) ||
      (type == FILE_MYERS_NSP) || /*(type == OBJECT_SCAN_FILE) ||*/
      (type == FILE_JPG) ||
      (type == FILE_DVID_OBJECT) ||
      (type == FILE_MC_STACK_RAW);
}

bool ZFileType::isImageFile(const std::string &filePath)
{
  return isImageFile(FileType(filePath));
}

bool ZFileType::isObjectFile(EFileType type)
{
  return (type == FILE_SWC) ||
      (type == FILE_SWC_NETWORK) ||
      (type == FILE_LOCSEG_CHAIN) ||
      (type == FILE_SYNAPSE_ANNOTATON) ||
      (type == FILE_FLYEM_NETWORK) ||
      (type == FILE_JSON) ||
      (type == FILE_V3D_APO) ||
      (type == FILE_V3D_MARKER) ||
      (type == FILE_RAVELER_BOOKMARK) ||
      (type == FILE_OBJECT_SCAN) ||
      (type == FILE_OBJECT_SCAN_ARRAY) ||
      (type == FILE_SPARSE_STACK) ||
      (type == FILE_MESH) ||
      (type == FILE_3D_GRAPH);
}

bool ZFileType::isObjectFile(const std::string &filePath)
{
  return isObjectFile(FileType(filePath));
}

bool ZFileType::isNeutubeOpenable(const std::string &filePath)
{
  return isNeutubeOpenable(FileType(filePath));
}

bool ZFileType::isNeutubeOpenable(EFileType type)
{
  bool openable = isImageFile(type);
  if (!openable) {
    openable = isObjectFile(type);
  }
  return openable;
}
