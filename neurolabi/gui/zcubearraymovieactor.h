#ifndef ZCUBEARRAYMOVIEACTOR_H
#define ZCUBEARRAYMOVIEACTOR_H

#include "zmovieactor.h"

class ZCubeArray;

class ZCubeArrayMovieActor : public ZMovieActor
{
public:
  ZCubeArrayMovieActor();

  virtual ~ZCubeArrayMovieActor();

  void setActor(ZCubeArray *cubeArray);

  virtual void hide();
  virtual void show();
  virtual void move(double t);

  virtual void pushColor();
  virtual void pushAlpha();
  virtual void pullColor();
  virtual void pullAlpha();

  virtual void reset();

private:
  ZCubeArray *m_cubeArray;
};

#endif // ZCUBEARRAYMOVIEACTOR_H
