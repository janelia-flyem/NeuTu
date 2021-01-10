#include "zpythonprocess.h"
#include <iostream>
#include <QFile>
#include <QDebug>
//#include "zerror.h"


ZPythonProcess::ZPythonProcess()
{
  m_pythonPath="python";
}


void ZPythonProcess::setPythonPath(const QString &python_path)
{
  m_pythonPath=python_path;
}

void ZPythonProcess::setWorkDir(const QString &dir)
{
  m_workDir = dir;
}

void ZPythonProcess::setScript(const QString &script)
{
  m_script = script;
}

void ZPythonProcess::addArg(const QString &arg)
{
  m_args.push_back(arg);
}

bool ZPythonProcess::runScript(const QString &script, bool parseOutput)
{
  if (m_pythonPath.isEmpty() || script.isEmpty()) {
    return false;
  }

  QStringList args;
  args << script;
  for (auto arg:m_args){
    args << arg;
  }
  m_process.start(m_pythonPath, args);

  if (!m_process.waitForStarted()){
    std::cerr << "lanuch python failed..." << std::endl;
    std::cerr << m_process.errorString().toStdString() << std::endl;
//    RECORD_ERROR_UNCOND(m_process.errorString().toStdString());
    return false;
  }
  while(!m_process.waitForFinished());

  // this parsing grabs the process output and does stuff to it, which in general
  //    you probably don't want; I don't believe it's used by anything in production,
  //    but for now, just add an optional parameter to turn it off if you prefer
  // (some scripts called with ZPythonProcess pass output via a file rather than
  //    stdout, so they don't care about this parsing)
  if (parseOutput) {
    parsePythonOutput();
  }

  return true;
}

bool ZPythonProcess::run(bool parseOutput)
{
  return runScript(m_script, parseOutput);
}

void ZPythonProcess::parsePythonOutput()
{
  m_output.clear();

  QString output = m_process.readAllStandardOutput();

  qDebug() << "in parsePythonOutput()";
  qDebug() << output.size();
  qDebug() << output;

  int startIndex = output.indexOf("@<json>") + 7;
  int endIndex = output.indexOf("</json>@");
  if (endIndex > startIndex) {
    m_output.decodeString(output.mid(startIndex, endIndex - startIndex).
                          toStdString().c_str());
  }
}

void ZPythonProcess::printOutputSummary() const
{
  if (m_output.isEmpty()) {
    std::cout << "No output." << std::endl;
  } else {
    m_output.print();
  }
}

QString ZPythonProcess::getRawOutput() {
    QString output = m_process.readAllStandardOutput();
    return output;
}

QString ZPythonProcess::getErrorOutput() {
    QString output = m_process.readAllStandardError();
    return output;
}

bool ZPythonProcess::findPython()
{
  return false;
}

void ZPythonProcess::printPythonPath()
{
  m_process.start("which", QStringList() << "python");
  m_process.waitForFinished();
  QString output = m_process.readAllStandardOutput();
}
