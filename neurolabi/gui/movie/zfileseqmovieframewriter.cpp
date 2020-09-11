#include "zfileseqmovieframewriter.h"

#include "neulib/core/stringbuilder.h"

ZFileseqMovieFrameWriter::ZFileseqMovieFrameWriter()
{
}

void ZFileseqMovieFrameWriter::write(const QImage &image, int frame)
{
  if (!m_pathPrefix.empty() && !image.isNull()) {
    std::string path =
        neulib::StringBuilder(m_pathPrefix).append(frame, m_padHint);
    image.save(QString::fromStdString(path));
  }
}
