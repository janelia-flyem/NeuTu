#include "zglobal.h"

#include "zintpoint.h"
#include "zpoint.h"

class ZGlobalData {

public:
  ZGlobalData();

  ZIntPoint m_stackPosition;
};

ZGlobalData::ZGlobalData()
{
  m_stackPosition.invalidate();
}


ZGlobal::ZGlobal()
{
  m_data = new ZGlobalData;
}

ZGlobal::~ZGlobal()
{
  delete m_data;
  m_data = NULL;
}

void ZGlobal::setStackPosition(int x, int y, int z)
{
  m_data->m_stackPosition.set(x, y, z);
}

ZIntPoint ZGlobal::getStackPosition() const
{
  return m_data->m_stackPosition;
}

void ZGlobal::setStackPosition(const ZIntPoint &pt)
{
  m_data->m_stackPosition = pt;
}

void ZGlobal::setStackPosition(const ZPoint &pt)
{
  setStackPosition(pt.toIntPoint());
}

void ZGlobal::clearStackPosition()
{
  m_data->m_stackPosition.invalidate();
}


