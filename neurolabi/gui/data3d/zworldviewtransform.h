#ifndef ZWORLDVIEWTRANSFORM_H
#define ZWORLDVIEWTRANSFORM_H

#include "common/neutudefs.h"
#include "geometry/zaffineplane.h"

class ZWorldViewTransform
{
public:
  ZWorldViewTransform();

  ZPoint transform(const ZPoint &pt) const;

  double getCutDepth() const;
  ZAffinePlane getCutPlane() const;
  neutu::EAxis getSliceAxis() const;

  /*!
   * \brief Set cut plane
   *
   * The slice axis will be set to neutu::EAxis::ARB automatically.
   */
  void setCutPlane(const ZAffinePlane &plane);

  /*!
   * \brief Set cut plane
   *
   * It will move the cut plane if the slice axis is eutu::EAxis::ARB.
   */
  void setCutPlane(neutu::EAxis sliceAxis, double cutDepth);

private:
  neutu::EAxis m_sliceAxis = neutu::EAxis::Z;

  ZAffinePlane m_cutPlane;
};

#endif // ZWORLDVIEWTRANSFORM_H
