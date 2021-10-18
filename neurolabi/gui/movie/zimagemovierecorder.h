#ifndef ZIMAGEMOVIERECORDER_H
#define ZIMAGEMOVIERECORDER_H

#include <memory>

#include "zimageframeshot.h"
#include "zmovieframewriter.h"

class ZImageMovieRecorder
{
public:
  ZImageMovieRecorder();

private:
  std::shared_ptr<ZImageFrameShot> m_fs;
  std::shared_ptr<ZMovieFrameWriter> m_writer;
  int m_currentFrame = 0;
};

#endif // ZIMAGEMOVIERECORDER_H
