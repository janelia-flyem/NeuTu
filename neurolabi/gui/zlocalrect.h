/**@file zlocalrect.h
 * @brief Local rectangle receptor
 * @author Ting Zhao
 */
#ifndef ZLOCALRECT_H
#define ZLOCALRECT_H

#include "zdirectionaltemplate.h"
#include "tz_r2_rect.h"

class ZLocalRect : public ZDirectionalTemplate
{
public:
  ZLocalRect();
  ZLocalRect(double x, double y, double z, double theta, double r);
//  ZLocalRect(const ZLocalRect &rect);

//  virtual const std::string& className() const;

public:
  void toLocalNeuroseg(Local_Neuroseg *locseg) const;
  Local_Neuroseg* toLocalNeuroseg() const override;

public:
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
  ZCuboid getBoundBox() const override;

public:
    void fitStack(const Stack *stack, Receptor_Fit_Workspace *ws) override;
    double fittingScore(const Stack *stack,
                ZDirectionalTemplate::FittingScoreOption option,
                Stack *mask = NULL) override;
    ZDirectionalTemplate* extend(Dlist_Direction_e direction = DL_FORWARD,
                                 double step = 0.5) override;
    void flip() override;
    bool hitTest(double x, double y, double z) override;
    ZPoint bottom() override;
    ZPoint center() override;
    ZPoint top() override;

private:
    double setFittingScoreOption(Stack_Fit_Score *fs,
                                 ZDirectionalTemplate::FittingScoreOption option);

private:
    Local_R2_Rect m_template;
};

#endif // ZLOCALRECT_H
