#include "zpythonprocess.h"
#include <iostream>
#include <QFile>
#include <QDebug>
#include "zerror.h"


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

bool ZPythonProcess::runScript(const QString &script)
{
  if (m_pythonPath.isEmpty() || script.isEmpty()) {
    return false;
  }

  QStringList args;
  args << script;
  for(auto arg:m_args){
    args<<arg;
  }
  m_process.start(m_pythonPath,args);

  if (!m_process.waitForStarted()){
    std::cerr<<"lanuch python failed..."<<std::endl;
    RECORD_ERROR_UNCOND(m_process.errorString().toStdString());
    return false;
  }
  while(!m_process.waitForFinished());

  parsePythonOutput();

  return true;
}

bool ZPythonProcess::run()
{
  return runScript(m_script);
}

void ZPythonProcess::parsePythonOutput()
{
  m_output.clear();

  QString output = m_process.readAllStandardOutput();


//  qDebug() << output.size();
//  qDebug() << output;

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
    std::cout << "No output.";
  } else {
    m_output.print();
  }
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
  std::cout << "Python path: " << output.toStdString() << std::endl;
}
