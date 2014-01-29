#include "zmatlabprocess.h"
#include <iostream>
#include <QFile>
#include <QDebug>
#include "zerror.h"

ZMatlabProcess::ZMatlabProcess()
{
}

void ZMatlabProcess::setWorkDir(const QString &dir)
{
  m_workDir = dir;
}

void ZMatlabProcess::setScript(const QString &script)
{
  m_script = script;
}

bool ZMatlabProcess::run()
{
  if (m_matlabPath.isEmpty() || m_script.isEmpty()) {
    return false;
  }

  QStringList args;
  args << "<" << m_script << "-nodesktop" << "-nosplash";
  m_process.start(m_matlabPath, args);

  if (!m_process.waitForFinished(-1)) {
    RECORD_ERROR_UNCOND(m_process.errorString().toStdString());
    return false;
  }

  parseMatlabOutput();

  return true;
}

void ZMatlabProcess::parseMatlabOutput()
{
  m_output.clear();

  QString output = m_process.readAllStandardOutput();

  qDebug() << output.size();
  qDebug() << output;

  int startIndex = output.indexOf("@<json>") + 7;
  int endIndex = output.indexOf("</json>@");
  if (endIndex > startIndex) {
    m_output.decodeString(output.mid(startIndex, endIndex - startIndex).
                          toStdString().c_str());
  }
}

void ZMatlabProcess::printOutputSummary() const
{
  if (m_output.isEmpty()) {
    std::cout << "No output.";
  } else {
    m_output.print();
  }
}

bool ZMatlabProcess::findMatlab()
{
#if defined(__APPLE__)
  QString matlabPath = "/Applications/MATLAB.app/bin/matlab";
  QFile matlab(matlabPath);
  if (matlab.exists()) {
    m_matlabPath = matlabPath;
    return true;
  }
#endif

  return false;
}
