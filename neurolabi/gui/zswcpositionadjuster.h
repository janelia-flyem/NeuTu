#ifndef ZSWCPOSITIONADJUSTER_H
#define ZSWCPOSITIONADJUSTER_H

#include "tz_image_lib_defs.h"
#include "zswctree.h"
#include "zswcpath.h"
#include "tz_sp_grow.h"
#include "common/neutudefs.h"
#include "zprogressable.h"

class ZSwcPositionAdjuster : public ZProgressable
{
public:
  ZSwcPositionAdjuster();
  virtual ~ZSwcPositionAdjuster();

public:
  inline void setSignal(
      Stack *signal,
      neutu::EImageBackground background = neutu::EImageBackground::DARK)
  {
    C_Stack::kill(m_mask);
    Kill_Sp_Grow_Workspace(m_workspace);
    m_mask = NULL;
    m_workspace = NULL;
    m_signal = signal;
    m_background = background;
  }

  void adjustPosition(ZSwcPath &swcPath);
  void adjustPosition(ZSwcTree &tree);

private:
  Stack *m_signal;
  Stack *m_mask;
  Sp_Grow_Workspace *m_workspace;
  neutu::EImageBackground m_background;
};

#endif // ZSWCPOSITIONADJUSTER_H
