#ifndef ZFLYEMBODYSPLITTER_H
#define ZFLYEMBODYSPLITTER_H

#include <QObject>

#include "tz_stdint.h"
#include "core/neutube_def.h"
#include "dvid/zdvidreader.h"

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
  virtual ~ZFlyEmBodySplitter();

  enum EState {
    STATE_NO_SPLIT, STATE_LOCAL_SPLIT, STATE_SPLIT, STATE_FULL_SPLIT
  };

  void setDvidTarget(const ZDvidTarget &target);

  uint64_t getBodyId() const;
  void setBodyId(uint64_t bodyId);

  void setBody(uint64_t bodyId, flyem::EBodyLabelType type, bool fromTar);

  flyem::EBodyLabelType getLabelType() const;
  EState getState() const;
  bool fromTar() const;
  void setFromTar(bool status);

  void runSplit();
  void runLocalSplit();
  void runFullSplit();

  void updateCachedMask(ZObject3dScan *obj);
  ZSparseStack* getBodyForSplit();

signals:
  void messageGenerated(const ZWidgetMessage&);

private:
  void notifyWindowMessageUpdated(const QString &message);
  void runSplit(ZFlyEmBody3dDoc *doc, flyem::EBodySplitRange rangeOption);
  void updateSplitState(flyem::EBodySplitRange rangeOption);
  void resetSplitState();

  void invalidateCache();
  void cacheBody(ZSparseStack *body);

//  void runSplit(ZFlyEmBody3dDoc *doc);
  template<typename T>
  T* getParentDoc() const;

private:
  uint64_t m_bodyId = 0; //Body for splitting
  flyem::EBodyLabelType m_labelType = flyem::EBodyLabelType::BODY;
  bool m_fromTar = false;
  EState m_state= STATE_NO_SPLIT;

  ZDvidReader m_reader;

  ZSparseStack *m_cachedObject = nullptr;
  uint64_t m_cachedBodyId = 0;
  flyem::EBodyLabelType m_cachedLabelType = flyem::EBodyLabelType::BODY;
};

#endif // ZFLYEMBODYSPLITTER_H
