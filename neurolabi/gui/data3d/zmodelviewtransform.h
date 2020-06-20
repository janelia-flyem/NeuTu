#ifndef ZWORLDVIEWTRANSFORM_H
#define ZWORLDVIEWTRANSFORM_H

#include "common/neutudefs.h"
#include "geometry/zaffineplane.h"

class ZJsonObject;

class ZModelViewTransform
{
public:
  ZModelViewTransform();

  ZPoint transform(const ZPoint &pt) const;
  void transform(double *x, double *y, double *z) const;

  ZPoint inverseTransform(double u, double v, double n = 0) const;
  ZPoint inverseTransform(const ZPoint &pt) const;

//  double getCutDepth() const;
  ZAffinePlane getCutPlane() const;
  neutu::EAxis getSliceAxis() const;
  ZPoint getCutPlaneNormal() const;
  ZPoint getCutCenter() const;

  /*!
   * \brief Transfrom the size of a box
   *
   * It shifts width, height and depth for when the slice axis is Y or Z. The
   * size becomes diagonal length along all dimensions when the slice axis is
   * ARB.
   */
  ZPoint transformBoxSize(const ZPoint &dim) const;

  /*!
   * \brief Set cut plane
   *
   * The slice axis will be set to neutu::EAxis::ARB automatically.
   */
  void setCutPlane(const ZAffinePlane &plane);
  void setCutPlane(const ZPoint &center, const ZPoint &v1, const ZPoint &v2);

  void setCutPlane(neutu::EAxis sliceAxis);

  /*!
   * \brief Set cut plane
   *
   * It will move the cut plane if the slice axis is eutu::EAxis::ARB.
   */
  void setCutPlane(neutu::EAxis sliceAxis, double cutDepth);

  void setCutPlane(neutu::EAxis sliceAxis, const ZPoint &cutCenter);

  void moveCutDepth(double d);
  void setCutDepth(const ZPoint &startPlane, double d);
  double getCutDepth(const ZPoint &startPlane) const;

  /*!
   * \brief Set the center of cut plane
   *
   * If the center is not compatible with the slice axis, the slice axis will
   * be set to neutu::EAxis::ARB.
   */
  void setCutCenter(double x, double y, double z);
  void setCutCenter(const ZPoint &pt);

  /*!
   * \brief Translate the cut center without changing the plane
   *
   * (\a du, \a dv) are the view-space coordinates of the translation.
   */
  void translateCutCenterOnPlane(double du, double dv);

  bool onSamePlane(const ZModelViewTransform &t) const;
  bool hasSamePlane(const ZModelViewTransform &t) const;

  ZJsonObject toJsonObject() const;

  bool operator==(const ZModelViewTransform &t) const;

  friend std::ostream& operator<< (
      std::ostream &stream, const ZModelViewTransform &t);

private:
  neutu::EAxis m_sliceAxis = neutu::EAxis::Z;

  ZAffinePlane m_cutPlane;
};

#endif // ZWORLDVIEWTRANSFORM_H
