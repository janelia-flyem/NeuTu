#ifndef Z3DSURFACEFILTER_H
#define Z3DSURFACEFILTER_H

#include <QObject>
#include "z3dgeometryfilter.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_graph.h"
#include "zwidgetsgroup.h"

class Z3DCubeRenderer;
class ZObject3d;

//
class Z3DCube
{
public:
    Z3DCube();
    ~Z3DCube();

public:
    int length;
    int x,y,z;
    glm::vec4 color;
};

//
class Z3DSurfaceFilter : public Z3DGeometryFilter
{
    Q_OBJECT

public:
    explicit Z3DSurfaceFilter();
    virtual ~Z3DSurfaceFilter();

    virtual void initialize();
    virtual void deinitialize();

    void prepareData();
    void addData(const Z3DCube &cube);

    virtual void process(Z3DEye);

    virtual void render(Z3DEye eye);

    ZWidgetsGroup *getWidgetsGroup();

    bool isReady(Z3DEye eye) const;

    void setVisible(bool v);

private:
    ZBoolParameter m_showCube;

    Z3DCubeRenderer *m_cubeRenderer;

    bool m_dataIsInvalid;

    ZWidgetsGroup *m_widgetsGroup;

};

#endif // Z3DSURFACEFILTER_H
