#ifndef ZFLYEMBODYSPLITTER_H
#define ZFLYEMBODYSPLITTER_H

#include <QObject>

#include "tz_stdint.h"
#include "neutube_def.h"

class ZStackDoc;
class ZFlyEmBody3dDoc;
class ZWidgetMessage;

/*!
 * \brief The class to replace ZFlyEmBodySplitProject.
 *
 * This class manages splitting in a document. It stores splitting enviroment
 * and controls the splitting action of a document.
 */
class ZFlyEmBodySplitter : public QObject
{
  Q_OBJECT

public:
  ZFlyEmBodySplitter(QObject *parent = NULL);
  virtual ~ZFlyEmBodySplitter() {}

  enum EState {
    STATE_NO_SPLIT, STATE_LOCAL_SPLIT, STATE_SPLIT, STATE_FULL_SPLIT
  };

  uint64_t getBodyId() const;
  void setBodyId(uint64_t bodyId);

  void setBody(uint64_t bodyId, flyem::EBodyLabelType type);

  flyem::EBodyLabelType getLabelType() const;
  EState getState() const;

  void runSplit();
  void runLocalSplit();
  void runFullSplit();

signals:
  void messageGenerated(const ZWidgetMessage&);

private:
  void notifyWindowMessageUpdated(const QString &message);
  void runSplit(ZFlyEmBody3dDoc *doc, flyem::EBodySplitRange rangeOption);
  void updateSplitState(flyem::EBodySplitRange rangeOption);
  void resetSplitState();

//  void runSplit(ZFlyEmBody3dDoc *doc);
  template<typename T>
  T* getParentDoc() const;

private:
  uint64_t m_bodyId = 0; //Body for splitting
  flyem::EBodyLabelType m_labelType = flyem::LABEL_BODY;
  EState m_state= STATE_NO_SPLIT;
};

#endif // ZFLYEMBODYSPLITTER_H
