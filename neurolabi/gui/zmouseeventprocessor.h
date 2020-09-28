#ifndef ZMOUSEEVENTPROCESSOR_H
#define ZMOUSEEVENTPROCESSOR_H

#include <QList>
#include "zmouseeventrecorder.h"
#include "zmouseeventmapper.h"
#include "common/zsharedpointer.h"
#include "geometry/zaffineplane.h"
#include "data3d/zsliceviewtransform.h"
#include "data3d/displayconfig.h"

class ZInteractiveContext;
class ZImageWidget;
class ZStackDoc;
class ZViewProj;

class ZMouseEventProcessor
{
public:
  ZMouseEventProcessor();
  ~ZMouseEventProcessor();

  void setInteractiveContext(ZInteractiveContext *context);
//  void setImageWidget(ZImageWidget *widget);
//  void setSliceAxis(neutu::EAxis axis);
//  void setArbSlice(const ZAffinePlane &ap);
  void setDocument(ZSharedPointer<ZStackDoc> doc);

//  neutu::EAxis getSliceAxis() const;

  const ZMouseEvent&
  process(QMouseEvent *event, ZMouseEvent::EAction action, int viewId);

  //void getCurrentMousePosition() const;
  //void getLastMousePosition() const;

  const ZMouseEventMapper& getMouseEventMapper(const ZMouseEvent &event) const;

  ZStackOperator getOperator() const;

//  ZPoint mapPositionFromWidgetToRawStack(
//      const ZIntPoint &pt, const ZViewProj &viewProj) const;
//  ZPoint mapPositionFromWidgetToRawStack(
//      int x, int y, int z, const ZViewProj &viewProj) const;
//  void mapPositionFromWidgetToRawStack(double *x, double *y) const;
//  void mapPositionFromWidgetToRawStack(
//      double *x, double *y, const ZViewProj &viewProj) const;

  const ZMouseEvent& getLatestMouseEvent() const;
  ZPoint getLatestStackPosition() const;
  const ZMouseEvent& getMouseEvent(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;
  ZPoint getStackPosition(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;
  ZPoint getLatestPosition(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action,
      neutu::data3d::ESpace space) const;
//  ZPoint getRawStackPosition(
//      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;

//  bool isPositionInStack(const ZMouseEvent &event) const;

  const ZStackDoc* getDocument() const;

  ZMouseEventRecorder& getRecorder() {
    return m_recorder;
  }

  const ZMouseEventRecorder& getRecorder() const {
    return m_recorder;
  }

private:
  void registerMapper();

private:
  ZMouseEventRecorder m_recorder;
  ZInteractiveContext *m_context;
//  ZImageWidget *m_imageWidget;
//  neutu::EAxis m_sliceAxis = neutu::EAxis::Z;
//  ZAffinePlane m_arbSlice; //Only valid for A_AXIS

  ZSharedPointer<ZStackDoc> m_doc;
  ZSliceViewTransform m_transform;

  ZMouseEvent m_emptyEvent;

  ZMouseEventMapper m_emptyMapper;
  ZMouseEventLeftButtonReleaseMapper m_leftButtonReleaseMapper;
  ZMouseEventLeftButtonPressMapper m_leftButtonPressMapper;
  ZMouseEventRightButtonPressMapper m_rightButtonPressMapper;
  ZMouseEventLeftRightButtonPressMapper m_bothButtonPressMapper;
  ZMouseEventRightButtonReleaseMapper m_rightButtonReleaseMapper;
  ZMouseEventMoveMapper m_moveMapper;
  ZMouseEventLeftButtonDoubleClickMapper m_leftButtonDoubleClickMapper;
  QList<ZMouseEventMapper*> m_mapperList;
};

#endif // ZMOUSEEVENTPROCESSOR_H
