#ifndef ZMATLABPROCESS_H
#define ZMATLABPROCESS_H

#include <QProcess>
#include "zjsonobject.h"

/*!
 * \brief The class of interfacing with Matlab
 */
class ZMatlabProcess
{
public:
  ZMatlabProcess();

  /*!
   * \brief Set working directory
   * \param dir The directory to store temporary results
   */
  void setWorkDir(const QString &dir);

  /*!
   * \brief Set the script
   *
   * \param The main Matlab script
   */
  void setScript(const QString &script);

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
  bool findMatlab();

  inline const ZJsonObject &getOutput() const {
    return m_output;
  }

  ZJsonObject &getOutput() {
    return m_output;
  }

public slots:
  void parseMatlabOutput();

private:
  QString m_matlabPath;
  QProcess m_process;
  ZJsonObject m_output;
  QString m_workDir;
  QString m_script;
};

#endif // ZMATLABPROCESS_H
