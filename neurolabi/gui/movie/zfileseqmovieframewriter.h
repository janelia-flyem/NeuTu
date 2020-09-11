#ifndef ZFILESEQMOVIEFRAMEWRITER_H
#define ZFILESEQMOVIEFRAMEWRITER_H

#include "zmovieframewriter.h"

class ZFileseqMovieFrameWriter : public ZMovieFrameWriter
{
public:
  ZFileseqMovieFrameWriter();

  void write(const QImage &image, int frame) override;

private:
  std::string m_pathPrefix;
  int m_padHint = 5;
};

#endif // ZFILESEQMOVIEFRAMEWRITER_H
