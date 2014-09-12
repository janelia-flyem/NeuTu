#ifndef ZMOUSEEVENTPROCESSOR_H
#define ZMOUSEEVENTPROCESSOR_H

#include <QRect>
#include "zmouseeventrecorder.h"
#include "zmouseeventmapper.h"

class ZInteractiveContext;
class ZImageWidget;

class ZMouseEventProcessor
{
public:
  ZMouseEventProcessor();

  void setInteractiveContext(ZInteractiveContext *context);
  void setImageWidget(ZImageWidget *widget);

  const ZMouseEvent&
  process(QMouseEvent *event, ZMouseEvent::EAction action, int z);

  void getCurrentMousePosition() const;
  void getLastMousePosition() const;

  const ZMouseEventMapper& getMouseEventMapper(const ZMouseEvent &event) const;

  ZMouseEventMapper::EOperation getOperation() const;

  ZPoint mapPositionFromWidgetToRawStack(const ZIntPoint &pt) const;
  ZPoint mapPositionFromWidgetToRawStack(int x, int y, int z) const;
  void mapPositionFromWidgetToRawStack(double *x, double *y) const;

  const ZMouseEvent& getLatestMouseEvent() const;
  const ZMouseEvent& getMouseEvent(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;

private:
  ZMouseEventRecorder m_recorder;
  ZInteractiveContext *m_context;
  ZImageWidget *m_imageWidget;

  ZMouseEventMapper m_emptyMapper;
  ZMouseEventLeftButtonReleaseMapper m_leftButtonReleaseMapper;
  ZMouseEventMoveMapper m_moveMapper;
};

#endif // ZMOUSEEVENTPROCESSOR_H
