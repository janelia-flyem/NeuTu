/**@file zlocalneuroseg.h
 * @brief Local neuroseg
 * @author Ting Zhao
 */
#ifndef ZLOCALNEUROSEG_H_
#define ZLOCALNEUROSEG_H_

#if defined(_QT_GUI_USED_)
#include <QFuture>
#endif

#include "zstackobject.h"
#include "tz_local_neuroseg.h"

class ZPointArray;
class ZStack;

class ZLocalNeuroseg : public ZStackObject
{
public:
  ZLocalNeuroseg(Local_Neuroseg *locseg, bool isOwner = true);
  virtual ~ZLocalNeuroseg() override;

//  virtual const std::string& className() const;

  double getHeight() const;
  double getRadius(double z) const;

public:
  static ZLocalNeuroseg& instance();

  static void display(const Local_Neuroseg *locseg, double z_scale,
                      QImage *image, int n = 0, Palette_Color color = Palette_Color::RED,
                      EDisplayStyle style = EDisplayStyle::NORMAL, int label = 0);
  ZCuboid getBoundBox() const override;

public:
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis axis) const override;
  void display(QImage *image, int n, Palette_Color color,
               EDisplayStyle style = EDisplayStyle::NORMAL, int label = 0) const;
  void display(ZPainter &painter, int sliceIndex, EDisplayStyle option,
               const QColor &color) const;
  using ZStackObject::display; // fix warning -Woverloaded-virtual

  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);

  void updateProfile(const Stack *stack, int option);
  bool hitMask(const ZStack *stack) const;
  bool hitMask(const Stack *stack) const;

  void topPosition(double pos[3]) const;
  void bottomPosition(double pos[3]) const;

  void asyncGenerateFilterStack() const;

  double getFitScore(const Stack *stack);

  ZPointArray sample(double xyStep, double zStep) const;

private:
  void generateFilterStack();
  void transform(coordinate_3d_t *points, size_t count) const;

private:
  Local_Neuroseg *m_locseg = nullptr;
  bool m_isOwner;
  double m_zscale;
  double *m_profile;

  Stack *m_filterStack;
  Field_Range m_fieldRange;
#if defined(_QT_GUI_USED_)
  mutable QFuture<void> m_future;
#endif
};

#endif
