#include "logging/zqslog.h"
#include <QProcess>

uint64_t getDedicatedVideoMemoryMB()
{
  uint64_t res = 0;

  QProcess cmd;
  cmd.start("grep", QStringList() << "-i" << "kByte" << "/var/log/Xorg.0.log");

  if (cmd.waitForFinished(-1)) {
    QString output(cmd.readAllStandardOutput());
    QStringList gpuInfos = output.split(QChar('\n'), QString::SkipEmptyParts);
    for (const auto& gpuInfo : gpuInfos) {
      LOG(INFO) << gpuInfo;
      QStringList gInfo = gpuInfo.split(QChar(' '), QString::SkipEmptyParts);
      for (int i = 0; i < gInfo.size(); ++i) {
        if (QString::compare(gInfo[i], "kBytes", Qt::CaseInsensitive) == 0 ||
            QString::compare(gInfo[i], "kByte", Qt::CaseInsensitive) == 0) {
          if (i > 0 && res == 0) {
            res = gInfo[i-1].toLongLong() / 1024;
          }
        }
      }
    }
  } else {
    LOG(ERROR) << cmd.readAllStandardError();
  }
  return res;
}
