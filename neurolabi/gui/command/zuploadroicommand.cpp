#include "zuploadroicommand.h"

#include <QDir>
#include <QDebug>

#include "dvid/zdvidwriter.h"
#include "flyem/flyemdatawriter.h"
#include "zdvidutil.h"

ZUploadRoiCommand::ZUploadRoiCommand()
{

}

int ZUploadRoiCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &/*config*/)
{
  if(input.empty() || output.empty()) {
    return 1;
  }


  ZDvidTarget target = dvid::MakeTargetFromUrlSpec(output);
//  target.setFromSourceString(output);

  ZDvidWriter writer;
  if (writer.open(target)) {
    writer.setAdmin(true);
    QDir dir(input[0].c_str());
    qDebug() << "Input data: " << dir;

    QStringList fileList = dir.entryList(QStringList() << "*.obj");
    qDebug() << fileList;
    for (const QString &meshFilename : fileList) {
      qDebug() << dir.filePath(meshFilename);
      QString roiName = meshFilename.left(
            meshFilename.length() - QString(".tif.obj").length());
      qDebug() << roiName;
      QString meshFilePath = dir.filePath(meshFilename);
      QString roiFilePath = dir.filePath(roiName + ".tif.sobj");
      if (!QFileInfo(meshFilePath).exists()) {
        qWarning() << meshFilePath << " does not exist. Abort!";
        break;
      }
      if (!QFileInfo(roiFilePath).exists()) {
        qWarning() << roiFilePath << " does not exist. Abort!";
        break;
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
