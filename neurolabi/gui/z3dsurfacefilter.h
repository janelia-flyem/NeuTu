#ifndef Z3DSURFACEFILTER_H
#define Z3DSURFACEFILTER_H

#include <QObject>
#include <vector>
#include "z3dgeometryfilter.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_graph.h"
#include "zwidgetsgroup.h"
#include "z3dcuberenderer.h"

class ZObject3d;

typedef std::vector<Z3DCube> CubeArrayType;

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
    void addData(ZCubeArray *cubes);
    void clearData();

    std::vector<double> boundBox();

    virtual void process(Z3DEye);

    virtual void render(Z3DEye eye);

    ZWidgetsGroup *getWidgetsGroup();

    bool isReady(Z3DEye eye) const;

    void setVisible(bool v);
    bool isVisible() const;

public slots:
    void updateSurfaceVisibleState();

private:
    ZBoolParameter m_showCube;

    std::vector<Z3DCube> m_cubeArray;
    std::vector<CubeArrayType> m_cubeArrayList;
    Z3DCubeRenderer *m_cubeRenderer;

    bool m_dataIsInvalid;

    ZWidgetsGroup *m_widgetsGroup;

};

#endif // Z3DSURFACEFILTER_H
