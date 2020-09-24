#ifndef ZSCROLLSLICESTRATEGY_H
#define ZSCROLLSLICESTRATEGY_H

class ZStackViewParam;
class ZStackView;

class ZScrollSliceStrategy
{
public:
  ZScrollSliceStrategy(ZStackView *view);
  virtual ~ZScrollSliceStrategy() {}

  void setView(ZStackView *view) {
    m_view = view;
  }

  int getViewId() const;

  void setRange(int minSlice, int maxSlice);

  virtual int scroll(int slice, int step) const;
  virtual int getValidSlice(int slice) const;
  virtual ZStackViewParam scroll(const ZStackViewParam &param, int step) const;
  virtual void scroll(int step);

protected:
  int m_minSlice;
  int m_maxSlice;
  ZStackView *m_view = nullptr;
};

#endif // ZSCROLLSLICESTRATEGY_H
