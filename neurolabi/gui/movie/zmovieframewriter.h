#ifndef ZMOVIEFRAMEWRITER_H
#define ZMOVIEFRAMEWRITER_H

#include <QImage>

class ZMovieFrameWriter
{
public:
  ZMovieFrameWriter();

  virtual void write(const QImage &image, int frame) = 0;
};

#endif // ZMOVIEFRAMEWRITER_H
