#ifndef VIS2D_UTILITIES_H
#define VIS2D_UTILITIES_H

class ZViewPlaneTransform;
class QTransform;

namespace neutu {

namespace vis2d {

QTransform GetPainterTransform(const ZViewPlaneTransform &t);

}

}

#endif // UTILITIES_H
