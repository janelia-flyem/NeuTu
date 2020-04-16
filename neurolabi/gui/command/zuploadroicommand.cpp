#include "zuploadroicommand.h"

#include <QDir>
#include <QDebug>

#include "dvid/zdvidwriter.h"
#include "flyem/flyemdatawriter.h"
#include "zdvidutil.h"
#include "zjsonobjectparser.h"

ZUploadRoiCommand::ZUploadRoiCommand()
{

}

namespace {

const size_t ROI_TYPE_DATA = 0;
const size_t ROI_TYPE_MESH = 1;
const QVector<QString> DATA_EXT = {"sobj", "obj"};

}

int ZUploadRoiCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  if(input.empty() || output.empty()) {
    return 1;
  }

  ZJsonObjectParser parser;

  bool uploadingMesh = parser.getValue(config, "mesh", true);
  bool uploadingData = parser.getValue(config, "data", true);

  if (uploadingMesh == false && uploadingData == false) {
    qWarning() << "No ROI will be loaded because all uploading options are off.";
    return 0;
  }

  size_t masterRoiType = ROI_TYPE_MESH;
  size_t chordRoiType = ROI_TYPE_DATA;
  if (uploadingMesh == false) {
    masterRoiType = ROI_TYPE_DATA;
    chordRoiType = ROI_TYPE_MESH;
  }

  ZDvidTarget target = dvid::MakeTargetFromUrlSpec(output);
//  target.setFromSourceString(output);

  ZDvidWriter writer;
  if (writer.open(target)) {
    writer.setAdmin(true);
    QDir dir(input[0].c_str());
    qDebug() << "Input data: " << dir;

    QStringList fileList =
        dir.entryList(QStringList() << "*." + DATA_EXT[masterRoiType]);
    qDebug() << fileList;
    for (const QString &masterFileName : fileList) {
      int extLength = DATA_EXT[masterRoiType].length() + 1;
      bool hasTifExt = false;
      if (masterFileName.endsWith(".tif." + DATA_EXT[masterRoiType])) {
        extLength += 4;
        hasTifExt = true;
      }
      QString roiName = masterFileName.left(
            masterFileName.length() - extLength);
      qDebug() << "ROI Name:" << roiName;

//      qDebug() << dir.filePath(meshFilename);
//      QString roiName = meshFilename.left(
//            meshFilename.length() - QString(".tif.obj").length());

      QString meshFilePath;

      if (uploadingMesh) {
        meshFilePath = dir.filePath(
              roiName + (hasTifExt ? ".tif." : ".") + DATA_EXT[ROI_TYPE_MESH]);
#ifdef _DEBUG_
        qDebug() << "Mesh file:" << meshFilePath;
#endif
        if (!QFileInfo(meshFilePath).exists()) {
          qWarning() << meshFilePath << " does not exist. Abort!";
          break;
        }
      }

      QString roiFilePath;

      if (uploadingData) {
        roiFilePath= dir.filePath(
              roiName + (hasTifExt ? ".tif." : ".") + DATA_EXT[ROI_TYPE_DATA]);
#ifdef _DEBUG_
        qDebug() << "Data file:" << roiFilePath;
#endif
        if (!QFileInfo(roiFilePath).exists()) {
          qWarning() << roiFilePath << " does not exist. Abort!";
          break;
        }
      }

      FlyEmDataWriter::UploadRoi(
            writer,
            roiName.toStdString(),
            roiFilePath.toStdString(),
            meshFilePath.toStdString());
    }
  } else {
    qWarning() << "Invalid DVID settings:" << input[0].c_str();
    return 1;
  }

  return 0;
}
