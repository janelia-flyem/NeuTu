#ifndef ZMESSAGE_H
#define ZMESSAGE_H

class QWidget;

class ZMessage
{
public:
  ZMessage();

public:
  bool isActive() const { return m_isActive; }
  bool isProcessed() const { return m_isProcessed; }

private:
  QWidget *m_originalSource;
  QWidget *m_currentSource;
  bool m_isProcessed;
  bool m_isActive;
};

#endif // ZMESSAGE_H
