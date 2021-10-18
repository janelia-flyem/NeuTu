#ifndef ZIMAGESLICEFACTORY_H
#define ZIMAGESLICEFACTORY_H

#include <memory>

class ZSliceCanvas;
class ZSparseStack;
class ZStack;
class ZModelViewTransform;
class ZPoint;

class ZImageSliceFactory
{
public:
  ZImageSliceFactory();


  static std::shared_ptr<ZSliceCanvas> Make(
      const ZStack &stack, const ZModelViewTransform &transform,
      double width, double height, std::shared_ptr<ZSliceCanvas> result);

  static std::shared_ptr<ZSliceCanvas> Make(
      const ZSparseStack &stack, const ZModelViewTransform &transform,
      double width, double height, std::shared_ptr<ZSliceCanvas> result);

  /*!
   * \brief Make a canvas from a slice of a stack along Z
   *
   * It makes canvas from the \a depth slice of \a stack. \a cutPlane, \a a,
   * \a b, \a zoom specifies the transformation of the canvas, which maps the
   * cut center of \a cutPlane to the anchor (\a a, \a b). \a zoom is the zoom
   * level of the canvas space to the model space, i.e. the transformation scale
   * is 1/2^\a zoom.
   */
  static std::shared_ptr<ZSliceCanvas> MakeXY(
      const ZStack &stack, int depth, const ZModelViewTransform &cutPlane,
      double a, double b, int zoom, std::shared_ptr<ZSliceCanvas> result);

  static std::shared_ptr<ZSliceCanvas> MakeXY(
      const ZSparseStack &stack, int depth, const ZModelViewTransform &cutPlane,
      double a, double b, int zoom, std::shared_ptr<ZSliceCanvas> result);
};

#endif // ZIMAGESLICEFACTORY_H
