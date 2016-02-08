/**@file zlocalneuroseg.h
 * @brief Local neuroseg
 * @author Ting Zhao
 */
#ifndef _ZLOCALNEUROSEG_H_
#define _ZLOCALNEUROSEG_H_

#if defined(_QT_GUI_USED_)
#include <QFuture>
#endif

#include "zstackobject.h"
#include "tz_local_neuroseg.h"

class ZLocalNeuroseg : public ZStackObject
{
public:
  ZLocalNeuroseg(Local_Neuroseg *locseg, bool isOwner = true);
  virtual ~ZLocalNeuroseg();

  virtual const std::string& className() const;

public:
  static ZLocalNeuroseg& instance();

  static void display(const Local_Neuroseg *locseg, double z_scale,
                      QImage *image, int n = 0, Palette_Color color = RED,
                      EDisplayStyle style = NORMAL, int label = 0);

public:
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis axis) const;
  void display(QImage *image, int n, Palette_Color color,
               EDisplayStyle style = NORMAL, int label = 0) const;
  void display(ZPainter &painter, int z, EDisplayStyle option,
               const QColor &color) const;
  using ZStackObject::display; // fix warning -Woverloaded-virtual

  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);

  void updateProfile(const Stack *stack, int option);

  void topPosition(double pos[3]) const;
  void bottomPosition(double pos[3]) const;

  void asyncGenerateFilterStack() const;

private:
  void generateFilterStack();

private:
  Local_Neuroseg *m_locseg;
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
