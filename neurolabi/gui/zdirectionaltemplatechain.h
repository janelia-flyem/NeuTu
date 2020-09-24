/**@file zdirectionaltemplatechain.h
 * @brief Chain of directional templates
 * @author Ting Zhao
 */

#ifndef ZDIRECTIONALTEMPLATECHAIN_H
#define ZDIRECTIONALTEMPLATECHAIN_H

#include <QList>
#include "zstackobject.h"
#include "tz_trace_defs.h"

class ZDirectionalTemplate;
class ZStack;
class ZLocsegChain;

class ZDirectionalTemplateChain : public ZStackObject
{
public:
    ZDirectionalTemplateChain();
    ~ZDirectionalTemplateChain();

//    virtual const std::string& className() const;

public:
    bool display(
        QPainter */*painter*/, const DisplayConfig &/*config*/) const override {
      return false;
    }
    /*
    virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                         neutu::EAxis sliceAxis) const override;
                         */
    void trace(const ZStack *stack, Trace_Workspace *tws);
    void append(ZDirectionalTemplate* dt);
    void prepend(ZDirectionalTemplate* dt);
    int hitTest(double x, double y, double z);
    ZLocsegChain* toLocsegChain();

    ZCuboid getBoundBox() const override;

private:
    QList<ZDirectionalTemplate*> m_chain;
};

#endif // ZDIRECTIONALTEMPLATECHAIN_H
