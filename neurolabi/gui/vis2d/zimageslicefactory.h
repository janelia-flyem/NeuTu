#ifndef ZIMAGESLICEFACTORY_H
#define ZIMAGESLICEFACTORY_H

class ZSliceCanvas;
class ZStack;
class ZModelViewTransform;

class ZImageSliceFactory
{
public:
  ZImageSliceFactory();


  static ZSliceCanvas* Make(
      const ZStack &stack, const ZModelViewTransform &transform,
      double width, double height, ZSliceCanvas *result);

};

#endif // ZIMAGESLICEFACTORY_H
