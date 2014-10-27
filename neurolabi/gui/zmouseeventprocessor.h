#ifndef ZMOUSEEVENTPROCESSOR_H
#define ZMOUSEEVENTPROCESSOR_H

#include <QList>
#include "zmouseeventrecorder.h"
#include "zmouseeventmapper.h"

class ZInteractiveContext;
class ZImageWidget;
class ZStackDoc;

class ZMouseEventProcessor
{
public:
  ZMouseEventProcessor();

  void setInteractiveContext(ZInteractiveContext *context);
  void setImageWidget(ZImageWidget *widget);
  void setDocument(ZStackDoc *doc);

  const ZMouseEvent&
  process(QMouseEvent *event, ZMouseEvent::EAction action, int z);

  //void getCurrentMousePosition() const;
  //void getLastMousePosition() const;

  const ZMouseEventMapper& getMouseEventMapper(const ZMouseEvent &event) const;

  ZStackOperator getOperator() const;

  ZPoint mapPositionFromWidgetToRawStack(const ZIntPoint &pt) const;
  ZPoint mapPositionFromWidgetToRawStack(int x, int y, int z) const;
  void mapPositionFromWidgetToRawStack(double *x, double *y) const;

  const ZMouseEvent& getLatestMouseEvent() const;
  ZPoint getLatestStackPosition() const;
  const ZMouseEvent& getMouseEvent(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;
  ZPoint getStackPosition(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;
  ZPoint getRawStackPosition(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;

  bool isPositionInStack(const ZMouseEvent &event) const;

  const ZStackDoc* getDocument() const;

  const ZMouseEventRecorder& getRecorder() const {
    return m_recorder;
  }

private:
  void registerMapper();

private:
  ZMouseEventRecorder m_recorder;
  ZInteractiveContext *m_context;
  ZImageWidget *m_imageWidget;
  ZStackDoc *m_doc;

  ZMouseEvent m_emptyEvent;

  ZMouseEventMapper m_emptyMapper;
  ZMouseEventLeftButtonReleaseMapper m_leftButtonReleaseMapper;
  ZMouseEventRightButtonReleaseMapper m_rightButtonReleaseMapper;
  ZMouseEventMoveMapper m_moveMapper;
  ZMouseEventLeftButtonDoubleClickMapper m_leftButtonDoubleClickMapper;
  QList<ZMouseEventMapper*> m_mapperList;
};

#endif // ZMOUSEEVENTPROCESSOR_H
