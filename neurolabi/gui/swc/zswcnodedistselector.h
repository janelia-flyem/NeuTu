#ifndef ZSWCNODESELECTOR_H
#define ZSWCNODESELECTOR_H

#include <vector>
#include "zswctreenodearray.h"

class ZSwcTree;

/*!
 * \brief A class of selected a set of nodes from a tree based on some criteria
 *
 * Still a prototype.
 */
class ZSwcNodeSelector
{
public:
  ZSwcNodeSelector();

  ZSwcTreeNodeArray select(const ZSwcTree &tree) const;

private:
  double m_minDistance;
};

#endif // ZSWCNODESELECTOR_H
