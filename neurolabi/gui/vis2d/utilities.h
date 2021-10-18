#ifndef VIS2D_UTILITIES_H
#define VIS2D_UTILITIES_H

class ZViewPlaneTransform;
class QTransform;
class ZSlice3dPainter;
class ZStack;
class ZSparseStack;
class QImage;
class ZAffineRect;
class ZSliceCanvas;
class ZSliceViewTransform;

namespace neutu{
namespace data3d {
class DisplayConfig;
}
}

namespace neutu {

namespace vis2d {

QTransform GetPainterTransform(const ZViewPlaneTransform &t);

ZSlice3dPainter Get3dSlicePainter(const neutu::data3d::DisplayConfig &config);
ZSliceViewTransform GetTransform(const neutu::data3d::DisplayConfig &config);

QImage GetSlice(const ZStack &stack, ZAffineRect &rect);
QImage GetSlice(const ZStack &stack, int z);
QImage GetSlice(const ZStack &stack, int &x0, int &y0, int &x1, int &y1, int z);
QImage GetSliceX(const ZStack &stack, int &x0, int &y0, int &x1, int &y1, int z);
QImage GetSliceY(const ZStack &stack, int &x0, int &y0, int &x1, int &y1, int z);
//ZSliceCanvas GetSliceCanvas(const ZStack &stack, const ZAffineRect &rect);

QImage GetSlice(
    const ZSparseStack &stack, int &x0, int &y0, int &x1, int &y1, int z);
QImage GetSlice(const ZSparseStack &stack, int z);
QImage GetSlice(const ZSparseStack &stack, ZAffineRect &rect);

}

}

#endif // UTILITIES_H
