#ifndef ZSYSTEMINFO_H
#define ZSYSTEMINFO_H

#include <QString>
#include <QDir>

class ZSystemInfo
{
public:
  static ZSystemInfo& instance();

  ZSystemInfo();

  void logOSInfo() const;

  bool is3DSupported() const
  { return m_glInitialized; }

  bool isStereoViewSupported() const
  { return m_stereoViewSupported; }

  void setStereoSupported(bool v)
  { m_stereoViewSupported = v; }

  // return false if failed
  virtual bool initializeGL();

  QString errorMessage() const
  { return m_errorMsg; }

  QString shaderPath(const QString& filename = "") const;

  QString fontPath(const QString& filename = "") const;

private:
  void detectOS();

protected:
  QString m_osString;

  QString m_errorMsg;

  QString m_fontPath;
  QString m_shaderPath;

  bool m_glInitialized = false;

  bool m_stereoViewSupported = false;
};

#endif // ZSYSTEMINFO_H
