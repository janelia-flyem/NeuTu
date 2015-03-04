#ifndef ZMESSAGE_H
#define ZMESSAGE_H

#include <string>
#include "zjsonobject.h"

class QWidget;

class ZMessage
{
public:
  ZMessage();

  enum EType {
    TYPE_FLYEM_SPLIT, TYPE_FLYEM_MERGE, TYPE_INFORMATION, TYPE_NULL
  };

public:
  bool isActive() const { return m_isActive; }
  bool isProcessed() const { return m_isProcessed; }

  inline EType getType() const { return m_type; }
  inline void setType(EType type) { m_type = type; }

  const ZJsonObject& getMessageBody() const { return m_body; }

  void deactivate();

  void setBodyEntry(std::string key, std::string value);

private:
  EType m_type;
  QWidget *m_originalSource;
  QWidget *m_currentSource;
  bool m_isProcessed;
  bool m_isActive;
  ZJsonObject m_body;
};

#endif // ZMESSAGE_H
