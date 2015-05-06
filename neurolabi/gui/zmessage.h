#ifndef ZMESSAGE_H
#define ZMESSAGE_H

#include <string>
#include "zjsonobject.h"

class QWidget;

class ZMessage
{
public:
  ZMessage(QWidget *source = NULL);

  enum EType {
    TYPE_FLYEM_SPLIT, TYPE_FLYEM_MERGE, TYPE_INFORMATION, TYPE_3D_VIS,
    TYPE_FLYEM_COARSE_3D_VIS,
    TYPE_FLYEM_SPLIT_VIEW_3D_BODY, TYPE_NULL
  };

public:
  bool isActive() const { return m_isActive; }
  bool isProcessed() const { return m_isProcessed; }

  inline EType getType() const { return m_type; }
  inline void setType(EType type) { m_type = type; }

  const ZJsonObject& getMessageBody() const { return m_body; }

  void deactivate();

  void setBodyEntry(const std::string &key, std::string value);
  void setBodyEntry(const std::string &key, uint64_t value);
  void setBodyEntry(const std::string &key, ZJsonObject obj);

  inline void setCurrentSource(const QWidget *source) {
    m_currentSource = const_cast<QWidget*>(source);
  }

  inline void setOriginalSource(const QWidget *source) {
    m_originalSource = const_cast<QWidget*>(source);
  }

  inline const QWidget* getCurrentSource() const { return m_currentSource; }
  inline const QWidget* getOriginalSource() const { return m_originalSource; }

private:
  EType m_type;
  QWidget *m_originalSource;
  QWidget *m_currentSource;
  bool m_isProcessed;
  bool m_isActive;
  ZJsonObject m_body;
};

#endif // ZMESSAGE_H
