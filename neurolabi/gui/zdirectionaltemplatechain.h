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

    virtual const std::string& className() const;

public:
    virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                         NeuTube::EAxis sliceAxis) const;
    void trace(const ZStack *stack, Trace_Workspace *tws);
    void append(ZDirectionalTemplate* dt);
    void prepend(ZDirectionalTemplate* dt);
    int hitTest(double x, double y, double z);
    ZLocsegChain* toLocsegChain();
private:
    QList<ZDirectionalTemplate*> m_chain;
};

#endif // ZDIRECTIONALTEMPLATECHAIN_H
