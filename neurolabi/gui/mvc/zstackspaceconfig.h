#ifndef ZSTACKSPACECONFIG_H
#define ZSTACKSPACECONFIG_H

#include "neutube_def.h"
#include "zpoint.h"

class ZStackSpaceConfig
{
public:
  ZStackSpaceConfig(neutube::EAxis axis);

private:
  neutube::EAxis m_axis = neutube::Z_AXIS;
  ZPoint m_v1;
  ZPoint m_v2;
};

#endif // ZSTACKSPACECONFIG_H
