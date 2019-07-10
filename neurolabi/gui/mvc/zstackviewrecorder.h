#ifndef ZSTACKVIEWRECORDER_H
#define ZSTACKVIEWRECORDER_H

#include <QString>

class ZStackView;

class ZStackViewRecorder
{
public:
  ZStackViewRecorder();

  enum class EMode {
    AUTO, MANUAL
  };

  void restart();
  void takeShot(ZStackView *view);

  void setPrefix(const QString prefix);
  void setMode(EMode mode);

  bool isAuto() const;

private:
  EMode m_mode = EMode::MANUAL;

  QString m_prefix;
  int m_currentIndex = 1;
};

#endif // ZSTACKVIEWRECORDER_H
