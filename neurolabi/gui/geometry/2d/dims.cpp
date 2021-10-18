#include "dims.h"

neutu::geom2d::Dims::Dims()
{

}

neutu::geom2d::Dims::Dims(double w, double h) : m_width(w), m_height(h)
{

}

double neutu::geom2d::Dims::getWidth() const
{
  return m_width;
}

double neutu::geom2d::Dims::getHeight() const
{
  return m_height;
}

void neutu::geom2d::Dims::setWidth(double w)
{
  m_width = w;
}

void neutu::geom2d::Dims::setHeight(double h)
{
  m_height = h;
}
