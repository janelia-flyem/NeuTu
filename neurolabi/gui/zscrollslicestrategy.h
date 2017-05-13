#ifndef ZSCROLLSLICESTRATEGY_H
#define ZSCROLLSLICESTRATEGY_H


class ZScrollSliceStrategy
{
public:
  ZScrollSliceStrategy();
  virtual ~ZScrollSliceStrategy() {}

  void setRange(int minSlice, int maxSlice);

  virtual int scroll(int slice, int step) const;
  virtual int getValidSlice(int slice) const;

protected:
  int m_minSlice;
  int m_maxSlice;
};

#endif // ZSCROLLSLICESTRATEGY_H
