#include "zdirectionaltemplate.h"
#include "tz_trace_utils.h"
#include "geometry/zpoint.h"
#include "geometry/zcuboid.h"

ZDirectionalTemplate::ZDirectionalTemplate()
{

}

ZDirectionalTemplate::ZDirectionalTemplate(const Trace_Record &tr)
{
  Trace_Record_Copy(&m_tr, &tr);
}

ZDirectionalTemplate::ZDirectionalTemplate(const ZDirectionalTemplate &dt) :
    ZStackObject(dt)
{
  *this = dt;
}

ZCuboid ZDirectionalTemplate::getBoundBox() const
{
  //Todo
  return ZCuboid();
}


ZDirectionalTemplate& ZDirectionalTemplate::operator =(
    const ZDirectionalTemplate &dt)
{
  Trace_Record_Copy(&m_tr, &dt.m_tr);

  return *this;
}
