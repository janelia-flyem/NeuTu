#ifndef ZFILESEQMOVIEFRAMEWRITER_H
#define ZFILESEQMOVIEFRAMEWRITER_H

#include "zmovieframewriter.h"

class ZFileseqMovieFrameWriter : public ZMovieFrameWriter
{
public:
  ZFileseqMovieFrameWriter();

  void setPathPrefix(const std::string &prefix);

  void write(const QImage &image, int frame) override;

private:
  std::string m_pathPrefix;
  std::string m_pathSuffix = ".tif";
  int m_padHint = 5;
};

#endif // ZFILESEQMOVIEFRAMEWRITER_H
