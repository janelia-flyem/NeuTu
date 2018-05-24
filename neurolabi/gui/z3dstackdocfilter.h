#ifndef Z3DSTACKDOCFILTER_H
#define Z3DSTACKDOCFILTER_H

#include "z3dboundedfilter.h"
#include "zstackdocptr.h"

/*!
 * \brief Obsolete.
 */
class Z3dStackDocFilter : public Z3DBoundedFilter
{
  Q_OBJECT

public:
  explicit Z3dStackDocFilter(
      Z3DGlobalParameters& globalPara, QObject* parent = nullptr);

//  virtual void setData(const ZStackDocPtr &doc) {};

};

#endif // Z3DSTACKDOCFILTER_H
