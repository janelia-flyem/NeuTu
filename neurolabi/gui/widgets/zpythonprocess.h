#ifndef ZPYTHONPROCESS_H
#define ZPYTHONPROCESS_H

#include <vector>
#include <QProcess>
#include "zjsonobject.h"

/*!
 * \brief The class of interfacing with python
 */
class ZPythonProcess
{
public:
  ZPythonProcess();

  /*!
   * \brief Set working directory
   * \param dir The directory to store temporary results
   */
  void setWorkDir(const QString &dir);

  void setPythonPath(const QString &python_path);
  /*!
   * \brief Set the script
   *
   * \param The main Matlab script
   */
  void setScript(const QString &script);

  void addArg(const QString &arg);
  /*!
   * \brief Run the matlab program
   *
   * \return true iff the Matlab program can be launched.
   */
  bool run();

  /*!
   * \brief Print output.
   */
  void printOutputSummary() const;

  /*!
   * \brief Find the executable path of Matlab
   *
   * \return true iff Matlab is found.
   */
  bool findPython();

  inline const ZJsonObject &getOutput() const {
    return m_output;
  }

  ZJsonObject &getOutput() {
    return m_output;
  }

public slots:
  void parsePythonOutput();

private:
  std::vector<QString> m_args;
  QString m_pythonPath;
  QProcess m_process;
  ZJsonObject m_output;
  QString m_workDir;
  QString m_script;
};


#endif // ZPYTHONPROCESS_H
