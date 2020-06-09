#ifndef ZIMAGESLICEFACTORY_H
#define ZIMAGESLICEFACTORY_H

class ZSliceCanvas;
class ZStack;
class ZModelViewTransform;
class ZPoint;

class ZImageSliceFactory
{
public:
  ZImageSliceFactory();


  static ZSliceCanvas* Make(
      const ZStack &stack, const ZModelViewTransform &transform,
      double width, double height, ZSliceCanvas *result);

  /*!
   * \brief Make a canvas from a slice of a stack along Z
   *
   * It makes canvas from the \a depth slice of \a stack. \a cutPlane, \a a,
   * \a b, \a zoom specifies the transformation of the canvas, which maps the
   * cut center of \a cutPlane to the anchor (\a a, \a b). \a zoom is the zoom
   * level of the canvas space to the model space, i.e. the transformation scale
   * is 1/2^\a zoom.
   */
  static ZSliceCanvas* MakeXY(
      const ZStack &stack, int depth, const ZModelViewTransform &cutPlane,
      double a, double b, int zoom, ZSliceCanvas *result);
};

#endif // ZIMAGESLICEFACTORY_H
